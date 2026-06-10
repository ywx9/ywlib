#pragma once

#include <map>

#include "yw/error.h"
#include "yw/slotset.h"
#include "yw/tuple.h"

#ifdef _WIN32
#include <shellapi.h>
#endif

#ifdef ywlib_header_name
#error "ywlib_header_name already defined unexpectedly"
#endif
#define ywlib_header_name "yw/argument.h"

namespace yw::errors {
define_error(argument_invalid_definition);
define_error(argument_invalid_argument);
define_error(argument_invalid_usage);
} // namespace yw::errors

namespace yw::argument {

/// name of application used in help text. This is overwritten in parsing.
inline string<char> name;

/// description of application used in help text
inline string<char> description;

/// epilog (ex: example usage) of application used in help text
inline string<char> epilog;

/// MARK: converter

template<typename T> struct converter {
  static std::expected<T, error> operator()(string_view<char>) {
    make_footprint;
    return std::unexpected(error(errors::argument_invalid_argument));
  }
};

template<> struct converter<string<char>> {
  static std::expected<string<char>, error> operator()(stringable<char> auto&& value) {
    make_footprint;
    return string<char>(value);
  }
};

template<> struct converter<bool> {
  static std::expected<bool, error> operator()(string_view<char> value) {
    make_footprint;
    if (value == "1" || value == "true" || value == "on" || value == "yes") return true;
    if (value == "0" || value == "false" || value == "off" || value == "no") return false;
    return std::unexpected(error(errors::argument_invalid_argument, format("Invalid bool: ", value)));
  }
  static std::expected<string<char>, error> operator()(bool value) {
    make_footprint;
    return vtos(value);
  }
};

template<typename T> requires(integral<T> && !is_bool<T>) struct converter<T> {
  static std::expected<T, error> operator()(string_view<char> value) {
    make_footprint;
    T out{};
    const auto* b = value.data();
    const auto* e = b + value.size();
    const auto r = std::from_chars(b, e, out);
    if (r.ec != std::errc() || r.ptr != e)
      return std::unexpected(error(errors::argument_invalid_argument, format("Invalid integer: ", value)));
    return out;
  }
  static std::expected<string<char>, error> operator()(T value) {
    make_footprint;
    return vtos(value);
  }
};

template<typename T> requires(float_type<T>) struct converter<T> {
  static std::expected<T, error> operator()(string_view<char> value) {
    make_footprint;
    string<char> s(value);
    char* end = nullptr;
    errno = 0;
    const auto v = std::strtold(s.c_str(), &end);
    if (errno != 0 || end != s.c_str() + s.size())
      return std::unexpected(error(errors::argument_invalid_argument, format("Invalid number: ", value)));
    return static_cast<T>(v);
  }
  static std::expected<string<char>, error> operator()(T value) {
    make_footprint;
    return vtos(value);
  }
};

/// MARK: handle

class handle {
public:
  enum class type : uint8_t { flag, option, positional };
  struct slot;
  class flag;
  template<typename T> class option;
  template<typename T> class positional;
  using slotid = slotset<slot>::slotid;

protected:
  slotid id;
  handle(slotid Id) : id(Id) {}
};

struct handle::slot {
  slotid id;
  string<char> key;
  string<char> metavar;
  string<char> description;
  handle::type type;
  bool required = false;
  bool multiple = false;
  bool specified = false;
  virtual ~slot() = default;
  virtual std::expected<void, error> parse_value(string_view<char>) {
    make_footprint;
    return std::unexpected(error(errors::argument_invalid_argument));
  }
  virtual std::optional<string<char>> default_value_string() const {
    make_footprint;
    return std::nullopt;
  }
};

namespace internal {
inline static bool parsed = false;
inline static slotset<handle::slot> slots;
inline static std::map<string<char>, handle::slotid> name_map;
inline static std::vector<handle::slotid> positionals;
} // namespace internal

/// MARK: positional

template<typename T> class handle::positional : public handle {
  positional(slotid Id) : handle(Id) {}

public:
  struct slot : handle::slot {
    std::optional<T> value;
    std::optional<T> default_value;

    static std::expected<positional, error> create(string<char> Metavar) {
      make_footprint;
      const auto id = internal::slots.add(std::make_unique<slot>());
      const auto sp = dynamic_cast<slot*>(internal::slots.get(id));
      if (!sp) return unexpected_error(errors::invalid_slotid, "Failed to create positional slot");
      sp->id = id;
      sp->key = uint_to_string(internal::positionals.size());
      sp->metavar = std::move(Metavar);
      sp->type = handle::type::positional;
      internal::positionals.push_back(id);
      return positional(id);
    }

    static slot& get(slotid Id) {
      make_footprint;
      const auto sp = dynamic_cast<slot*>(internal::slots.get(Id));
      if (!sp) fatal_error(error(errors::operation_failed, "Failed to get positional slot"));
      return *sp;
    }

    std::expected<void, error> parse_value(std::string_view Value) override {
      make_footprint;
      if (auto res = converter<T>{}(Value); !res) {
        yw::error& e = res.error();
        e.message = format(e.message, " at #", key);
        return std::unexpected(res.error());
      } else value = std::move(*res);
      specified = true;
      return {};
    }

    std::optional<string<char>> default_value_string() const override {
      if (default_value.has_value()) {
        if (auto res = converter<T>{}(*default_value); res) return *res;
        else return std::nullopt;
      } else return std::nullopt;
    }
  };

  auto& description(string<char> Desc) {
    make_footprint;
    return slot::get(id).description = std::move(Desc), *this;
  }

  auto& required(bool Required = true) {
    make_footprint;
    return slot::get(id).required = Required, *this;
  }
  auto& default_value(T Value) {
    make_footprint;
    return slot::get(id).default_value = std::move(Value), *this;
  }

  const auto& value() const {
    make_footprint;
    if (!internal::parsed) fatal_error(error(errors::argument_invalid_usage, "Call before parsing"));
    auto& s = slot::get(id);
    if (s.specified) return *s.value;
    if (!s.default_value.has_value()) fatal_error(error(errors::argument_invalid_usage, "No value available"));
    return *s.default_value;
  }

  bool has_value() const {
    make_footprint;
    if (!internal::parsed) fatal_error(error(errors::argument_invalid_usage, "Call before parsing"));
    auto& s = slot::get(id);
    return s.specified || s.default_value.has_value();
  }

  bool specified() const {
    make_footprint;
    if (!internal::parsed) fatal_error(error(errors::argument_invalid_usage, "Call before parsing"));
    return slot::get(id).specified;
  }
};

template<typename T> handle::positional<T> positional(string<char> Metavar) {
  make_footprint;
  auto res = handle::positional<T>::slot::create(std::move(Metavar));
  if (!res) fatal_error(res.error());
  return std::move(*res);
}

/// MARK: option

template<typename T> class handle::option : public handle {
  option(slotid Id) : handle(Id) {}

public:
  struct slot : handle::slot {
    std::vector<T> values;
    std::optional<T> default_value;

    static std::expected<option, error> create(string<char> Key) {
      make_footprint;
      const auto id = internal::slots.add(std::make_unique<slot>());
      const auto [it, b] = internal::name_map.emplace(Key, id);
      if (!b) return unexpected_error(errors::argument_invalid_definition, format("Duplicate key: ", Key));
      const auto sp = dynamic_cast<slot*>(internal::slots.get(id));
      if (!sp) return unexpected_error(errors::operation_failed, "Failed to create option slot");
      sp->id = id;
      sp->key = std::move(Key);
      sp->type = handle::type::option;
      return option(id);
    }

    static slot& get(slotid Id) {
      make_footprint;
      const auto sp = dynamic_cast<slot*>(internal::slots.get(Id));
      if (!sp) fatal_error(error(errors::operation_failed, "Failed to get option slot"));
      return *sp;
    }

    std::expected<void, error> parse_value(string_view<char> Value) override {
      make_footprint;
      if (auto res = converter<T>{}(Value); !res) {
        yw::error& e = res.error();
        e.message = format(e.message, " as '", key, "'");
        return unexpected_error(res.error());
      } else if (multiple || values.empty()) values.push_back(std::move(*res));
      specified = true;
      return {};
    }

    std::optional<string<char>> default_value_string() const override {
      if (default_value.has_value()) {
        if (auto res = converter<T>{}(*default_value); res) return *res;
        else return std::nullopt;
      } else return std::nullopt;
    }
  };

  auto& metavar(string<char> Metavar) {
    make_footprint;
    return slot::get(id).metavar = std::move(Metavar), *this;
  }
  auto& description(string<char> Desc) {
    make_footprint;
    return slot::get(id).description = std::move(Desc), *this;
  }
  auto& required(bool Required = true) {
    make_footprint;
    return slot::get(id).required = Required, *this;
  }
  auto& multiple(bool Multiple = true) {
    make_footprint;
    return slot::get(id).multiple = Multiple, *this;
  }
  auto& default_value(T Value) {
    make_footprint;
    return slot::get(id).default_value = std::move(Value), *this;
  }

  auto& alias(string<char> Alias) {
    make_footprint;
    const auto [it, b] = internal::name_map.emplace(Alias, id);
    if (!b) fatal_error(error(errors::argument_invalid_definition, format("Duplicate key: ", Alias)));
    return *this;
  }

  const auto& value() const {
    make_footprint;
    if (!internal::parsed) fatal_error(error(errors::argument_invalid_usage, "Call before parsing"));
    auto& s = slot::get(id);
    if (s.specified) return s.values.front();
    if (!s.default_value.has_value()) fatal_error(error(errors::argument_invalid_usage, "No value available"));
    return *s.default_value;
  }

  const auto& values() const {
    make_footprint;
    if (!internal::parsed) fatal_error(error(errors::argument_invalid_usage, "Call before parsing"));
    return slot::get(id).values;
  }

  bool has_value() const {
    make_footprint;
    if (!internal::parsed) fatal_error(error(errors::argument_invalid_usage, "Call before parsing"));
    const auto& s = slot::get(id);
    return s.specified || s.default_value.has_value();
  }

  bool specified() const {
    make_footprint;
    if (!internal::parsed) fatal_error(error(errors::argument_invalid_usage, "Call before parsing"));
    return slot::get(id).specified;
  }
};

template<typename T> handle::option<T> option(string<char> Name) {
  make_footprint;
  auto res = handle::option<T>::slot::create(std::move(Name));
  if (!res) fatal_error(res.error());
  return std::move(*res);
}

/// MARK: flag

class handle::flag : public handle {
  flag(slotid Id) : handle(Id) {}

public:
  struct slot : handle::slot {
    static std::expected<flag, error> create(string<char> Key) {
      make_footprint;
      const auto id = internal::slots.add(std::make_unique<slot>());
      const auto [it, b] = internal::name_map.emplace(Key, id);
      if (!b) return std::unexpected(error(errors::argument_invalid_definition, format("Duplicate key: ", Key)));
      const auto sp = dynamic_cast<slot*>(internal::slots.get(id));
      if (!sp) return std::unexpected(error(errors::operation_failed, "Failed to create flag slot"));
      sp->id = id;
      sp->key = std::move(Key);
      sp->type = handle::type::flag;
      return flag(id);
    }

    static slot& get(slotid Id) {
      make_footprint;
      const auto sp = dynamic_cast<slot*>(internal::slots.get(Id));
      if (!sp) fatal_error(error(errors::operation_failed, "Failed to get flag slot"));
      return *sp;
    }

    std::expected<void, error> parse_value(string_view<char> value) override { return {}; }
  };

  auto& description(string<char> Desc) {
    make_footprint;
    return slot::get(id).description = std::move(Desc), *this;
  }

  auto& alias(string<char> Alias) {
    make_footprint;
    const auto [it, b] = internal::name_map.emplace(Alias, id);
    if (!b) fatal_error(error(errors::argument_invalid_definition, format("Duplicate key: ", Alias)));
    return *this;
  }

  bool value() const {
    make_footprint;
    if (!internal::parsed) fatal_error(error(errors::argument_invalid_usage, "Call before parsing"));
    return slot::get(id).specified;
  }

  bool has_value() const {
    make_footprint;
    if (!internal::parsed) fatal_error(error(errors::argument_invalid_usage, "Call before parsing"));
    return true;
  }

  bool specified() const { return value(); }
};

inline handle::flag flag(string<char> Name) {
  make_footprint;
  auto res = handle::flag::slot::create(std::move(Name));
  if (!res) fatal_error(res.error());
  return std::move(*res);
}

/// MARK: print_help

namespace internal {
inline string<char> default_option_metavar(string_view<char> key) {
  while (!key.empty() && key.front() == '-') key.remove_prefix(1);
  string<char> out;
  out.reserve(key.size());
  for (const char c : key) {
    if (c == '-') out.push_back('_');
    else out.push_back(char(std::toupper(static_cast<unsigned char>(c))));
  }
  return out.empty() ? "VALUE" : out;
}
} // namespace internal

inline void print_help() {
  const string<char> app_name = name.empty() ? "app" : name;

  auto default_option_metavar = [](string_view<char> key) {
    while (!key.empty() && key.front() == '-') key.remove_prefix(1);

    string<char> out;
    out.reserve(key.size());

    for (const char c : key) {
      if (c == '-') out.push_back('_');
      else out.push_back(char(std::toupper(static_cast<unsigned char>(c))));
    }

    return out.empty() ? string<char>("VALUE") : out;
  };

  auto display_metavar = [&](const handle::slot& slot) {
    if (!slot.metavar.empty()) return slot.metavar;

    if (slot.type == handle::type::option) { return default_option_metavar(slot.key); }

    if (slot.type == handle::type::positional) { return string<char>("ARG"); }

    return string<char>{};
  };

  auto names_of = [](const handle::slot& slot) {
    std::vector<string<char>> names;
    names.push_back(slot.key);
    std::vector<string<char>> aliases;
    for (const auto& [name, id] : internal::name_map)
      if (id == slot.id && name != slot.key) aliases.push_back(name);
    std::ranges::sort(aliases);
    names.insert(names.end(), aliases.begin(), aliases.end());
    return names;
  };

  auto primary_usage_name = [](const handle::slot& slot) { return slot.key; };

  auto option_usage = [&](const handle::slot& slot) {
    string<char> s = primary_usage_name(slot);
    if (slot.type == handle::type::option) {
      s += " ";
      s += display_metavar(slot);
    }
    if (slot.multiple) s = format("[", s, "]...");
    else if (!slot.required) s = format("[", s, "]");
    return s;
  };

  auto option_lhs = [&](const handle::slot& slot) {
    const auto names = names_of(slot);
    string<char> lhs;
    for (const auto& n : names) {
      if (!lhs.empty()) lhs += ", ";
      lhs += n;
    }
    if (lhs.empty()) lhs = slot.key;
    if (slot.type == handle::type::option) {
      lhs += " ";
      lhs += display_metavar(slot);
    }
    return lhs;
  };

  auto append_note = [](string<char>& rhs, string_view<char> note) {
    if (!rhs.empty()) rhs += " ";
    rhs += note;
  };

  auto detail_text = [&](const handle::slot& slot) {
    string<char> rhs;
    if (!slot.description.empty()) { rhs += slot.description; }
    if (slot.required) { append_note(rhs, "(required)"); }
    if (slot.multiple) { append_note(rhs, "(multiple)"); }
    if (auto dv = slot.default_value_string(); dv.has_value()) append_note(rhs, format("(default: ", *dv, ")"));
    return rhs;
  };

  auto print_entry = [](string_view<char> lhs, string_view<char> rhs) {
    if (rhs.empty()) print("  {}", lhs);
    else print("  {:<24} {}", lhs, rhs);
  };

  if (!name.empty()) print("{}", name);
  if (!description.empty()) print("{}", description);
  if (!name.empty() || !description.empty()) print("");

  auto usage = format("Usage: ", app_name);

  for (const auto& slot : internal::slots) {
    if (slot.type == handle::type::positional) continue;
    usage += " ";
    usage += option_usage(slot);
  }

  for (const auto pid : internal::positionals) {
    const auto* p = internal::slots.get(pid);
    if (!p) continue;

    const auto mv = display_metavar(*p);

    if (p->required) usage += format(" ", mv);
    else usage += format(" [", mv, "]");
  }

  print(usage);

  bool has_value_options = false;
  for (const auto& slot : internal::slots) {
    if (slot.type == handle::type::option) {
      has_value_options = true;
      break;
    }
  }

  if (has_value_options) { print("Note: Options with values may be written as either --key VALUE or --key=VALUE."); }

  print("");

  print("Options:");
  print_entry("--help, -h", "Show this help message and exit");

  for (const auto& slot : internal::slots) {
    if (slot.type == handle::type::positional) continue;

    const auto lhs = option_lhs(slot);
    const auto rhs = detail_text(slot);
    print_entry(lhs, rhs);
  }

  if (!internal::positionals.empty()) {
    print("");
    print("Positional arguments:");

    for (const auto pid : internal::positionals) {
      const auto* p = internal::slots.get(pid);
      if (!p) continue;

      const auto lhs = display_metavar(*p);
      const auto rhs = detail_text(*p);
      print_entry(lhs, rhs);
    }
  }

  if (!epilog.empty()) {
    print("");
    print("{}", epilog);
  }
}

/// MARK: parse helper

namespace internal {

inline std::vector<string<char>> collect_argv(int argc, char** argv, string<char>& program_name) {
  make_footprint;
  std::vector<string<char>> args;
#if defined(_WIN32) || defined(_WIN64)
  int c = 0;
  auto v = ::CommandLineToArgvW(::GetCommandLineW(), &c);
  if (!v) fatal_win32_error(CommandLineToArgvW);
  args.reserve(static_cast<size_t>(c));
  for (int i = 0; i < c; ++i) args.emplace_back(unicode<char>(std::wstring_view(v[i])));
  ::LocalFree(v);
#else
  args.reserve(static_cast<size_t>(argc));
  for (int i = 0; i < argc; ++i) args.emplace_back(argv[i]);
#endif
  if (!args.empty() && name.empty()) program_name = unicode<char>(path(args.front().view()).stem().native());
  return args;
}

inline bool is_option_token(const string_view<char> tok) { return tok.size() > 1 && tok[0] == '-'; }

inline std::optional<std::pair<string_view<char>, string_view<char>>> split_eq(string_view<char> tok) {
  if (auto i = tok.find('='); i == string_view<char>::npos) return std::nullopt;
  else if (i == 0) return std::nullopt;
  else return std::pair<string_view<char>, string_view<char>>{tok.substr(0, i), tok.substr(i + 1)};
}

inline bool is_negative_number_token(string_view<char> tok) {
  return !(tok.size() < 2 || tok[0] != '-') && (is_digit(tok[1]) || tok[1] == '.');
}

inline void argument_error(ministr<char> msg) {
  make_footprint;
  print_help();
  fatal_error(error(errors::argument_invalid_argument, std::move(msg)));
}
} // namespace internal

/// MARK: parse

inline void parse(int argc, char** argv) {
  make_footprint;
  if (internal::parsed) fatal_error(error(errors::argument_invalid_usage, "Arguments have already been parsed"));
  auto args = internal::collect_argv(argc, argv, argument::name);
  bool after_double_dash = false;
  size_t positional_index = 0;
  for (size_t i = 1; i < args.size(); ++i) {
    const string_view<char> tok = args[i];
    if (!after_double_dash && tok == "--") {
      after_double_dash = true;
      continue;
    }
    if (!after_double_dash && internal::is_option_token(tok)) {
      string_view<char> key = tok;
      string_view<char> value{};
      bool specified = false;
      if (auto kv = internal::split_eq(tok)) {
        key = kv->first;
        value = kv->second;
        specified = true;
      }
      auto it = internal::name_map.find(string<char>(key));
      if (it == internal::name_map.end()) {
        if (key == "--help" || key == "-h") {
          print_help();
          std::exit(0);
        }
        internal::argument_error(format("Unknown option: ", key));
      }
      const auto sp = internal::slots.get(it->second);
      if (!sp) internal::argument_error(format("Unknown option: ", key));

      if (sp->type == handle::type::option) {
        if (!specified && i + 1 < args.size()) {
          const auto next = args[i + 1];
          if (!internal::is_option_token(next) || internal::is_negative_number_token(next))
            value = next, specified = true, ++i;
        }
        if (!specified) internal::argument_error(format("Missing value for option: ", key));
        if (auto res = sp->parse_value(value); !res) print_help(), fatal_error(res.error());
      } else if (specified) internal::argument_error(format("Unexpected value for flag: ", key));
      else sp->specified = true;
      continue;
    }

    if (positional_index < internal::positionals.size()) {
      const auto sp = internal::slots.get(internal::positionals[positional_index]);
      if (!sp) internal::argument_error(format("Unknown, positional argument: ", tok));
      if (auto res = sp->parse_value(tok); !res) fatal_error(res.error());
      ++positional_index;
    } else internal::argument_error(format("Unexpected positional argument: ", tok));
  }

  bool missing = false;
  string<char> missing_message = "Missing required options";
  for (const auto& slot : internal::slots) {
    if (slot.required && !slot.specified) {
      if (!missing) {
        missing = true;
        missing_message += format(": ", slot.key);
      } else missing_message += format(", ", slot.key);
    }
  }
  if (missing) internal::argument_error(missing_message);
  internal::parsed = true;
}
} // namespace yw::argument

#undef ywlib_header_name
