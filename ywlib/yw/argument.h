#pragma once

#include <yw/file.h>
#include <yw/slotset.h>
#include <yw/tuple.h>

#include <map>

#ifdef _WIN32
#include <shellapi.h>
#endif

namespace yw::errors {
inline constexpr error::kind invalid_command_line_argument{"invalid command line argument"};
inline constexpr error::kind missing_required_argument{"missing required argument"};
} // namespace yw::errors

namespace yw::argument {

/// name of application used in help text. This is overwritten in parsing.
inline string<char> program_name;

/// description of application used in help text
inline string<char> description;

/// epilog (ex: example usage) of application used in help text
inline string<char> epilog;

/// MARK: converter

template<typename T> struct converter {
  static std::expected<T, error> operator()(string_view<char>) {
    return std::unexpected(error(errors::invalid_command_line_argument));
  }
};

template<> struct converter<string<char>> {
  static std::expected<string<char>, error> operator()(stringable<char> auto&& value) { return string<char>(value); }
};

template<> struct converter<bool> {
  static std::expected<bool, error> operator()(string_view<char> value) {
    if (value == "1" || value == "true" || value == "on" || value == "yes") return true;
    if (value == "0" || value == "false" || value == "off" || value == "no") return false;
    return std::unexpected(error(errors::invalid_command_line_argument, format("Invalid bool: ", value)));
  }
  static std::expected<string<char>, error> operator()(bool value) { return vtos(value); }
};

template<typename T> requires(integral<T> && !is_bool<T>) struct converter<T> {
  static std::expected<T, error> operator()(string_view<char> value) {
    T out{};
    const auto* b = value.data();
    const auto* e = b + value.size();
    const auto r = std::from_chars(b, e, out);
    if (r.ec != std::errc() || r.ptr != e)
      return std::unexpected(error(errors::invalid_command_line_argument, format("Invalid integer: ", value)));
    return out;
  }
  static std::expected<string<char>, error> operator()(T value) { return vtos(value); }
};

template<typename T> requires(float_type<T>) struct converter<T> {
  static std::expected<T, error> operator()(string_view<char> value) {
    string<char> s(value);
    char* end = nullptr;
    errno = 0;
    const auto v = std::strtold(s.c_str(), &end);
    if (errno != 0 || end != s.c_str() + s.size())
      return std::unexpected(error(errors::invalid_command_line_argument, format("Invalid number: ", value)));
    return static_cast<T>(v);
  }
  static std::expected<string<char>, error> operator()(T value) { return vtos(value); }
};

/// MARK: handle

class handle : public general_handle {
protected:
  inline static std::map<string<char>, general_slotid> name_map;
  inline static std::vector<general_slotid> positionals;

public:
  enum class type : uint8_t { flag, option, positional };
  class flag;
  template<typename T> class option;
  template<typename T> class positional;

  struct slot : general_handle::slot {
    string<char> key;                  // flag, option, positional
    string<char> metavar;              // positional
    string<char> description;          // flag, option, positional
    string<char> value_string;         // option, positional
    string<char> default_value_string; // option, positional
    std::vector<string<char>> aliases; // flag, option
    handle::type type;
    bool required = false;
    bool multiple = false;
    bool specified = false;
  };

  using general_handle::general_handle;
};

namespace internal {
inline static bool parsed = false;
} // namespace internal

/// MARK: positional

template<typename T> class handle::positional : public handle {
public:
  struct slot : handle::slot {
    static const slot empty_slot;
    std::optional<T> value;
    std::optional<T> default_value;

    std::expected<void, error> init(string<char>&& Metavar) {
      key = uint_to_string(positionals.size());
      metavar = std::move(Metavar);
      type = handle::type::positional;
      positionals.push_back(id);
      return {};
    }

    std::expected<void, error> update_value() {
      if (value.has_value()) return {};
      if (!specified) {
        if (default_value.has_value()) value = *default_value;
        else return std::unexpected(error(errors::missing_required_argument));
      } else if (auto res = converter<T>()(value_string); !res) {
        auto& e = res.error();
        e.message(format(e.message(), " at #", key));
        return e.relay();
      } else value = std::move(*res);
      return {};
    }
  };

  positional(string<char>&& Metavar, const source_line& sl = here()) {
    if (const auto sp = create_slot<positional>(sl); !sp) error(error(errors::slot_creation_failed)).go_off(sl); // fatal
    else if (auto res = sp->init(std::move(Metavar)); !res) res.error().add_footprint().go_off(sl); // fatal
    else _id = sp->id;
  }

  template<typename Self> auto&& description(this Self&& self, string<char> Desc) noexcept
    requires(!is_const<remove_ref<Self>>) {
    const auto sp = self.get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out(); // warning
    else sp->description = std::move(Desc);
    return static_cast<Self&&>(self);
  }

  template<typename Self> auto&& required(this Self&& self, bool Required = true) noexcept
    requires(!is_const<remove_ref<Self>>) {
    const auto sp = self.get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out(); // warning
    else sp->required = Required;
    return static_cast<Self&&>(self);
  }

  template<typename Self> auto&& default_value(this Self&& self, T Value) noexcept
    requires(!is_const<remove_ref<Self>>) {
    const auto sp = self.get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off(); // fatal
    if (auto res = converter<T>{}(Value)) {
      sp->default_value_string = std::move(*res);
      sp->default_value = std::move(Value);
    } else res.error().add_footprint().fizzle_out(sp->source_line); // warning
    return static_cast<Self&&>(self);
  }

  const T& value() const {
    if (!internal::parsed) error(errors::invalid_operation, "value required before parsing").go_off(); // fatal
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off(); // fatal
    if (auto res = sp->update_value(); !res) res.error().add_footprint().go_off(sp->source_line); // fatal
    return *(sp->value);
  }

  bool has_value() const {
    const auto sp = get_slot(this);
    return sp && (sp->specified || sp->default_value.has_value());
  }

  bool specified() const {
    const auto sp = get_slot(this);
    return sp && sp->specified;
  }
};

template<typename T>
handle::positional<T> positional(string<char> Metavar, const source_line& sl = here()) {
  return handle::positional<T>(std::move(Metavar), sl);
}

/// MARK: option

template<typename T> class handle::option : public handle {
public:
  struct slot : handle::slot {
    static const slot empty_slot;
    std::optional<std::vector<T>> values;
    std::optional<T> default_value;

    std::expected<void, error> init(string<char> Key) {
      const auto [it, b] = name_map.emplace(Key, id);
      if (!b) return std::unexpected(error(errors::operation_failed, format("Duplicate key: ", Key)));
      key = std::move(Key);
      type = handle::type::option;
      return {};
    }

    std::expected<void, error> update_value() {
      if (values.has_value()) return {};
      if (!specified) {
        if (default_value.has_value()) values = std::vector<T>{*default_value};
        else return std::unexpected(error(errors::missing_required_argument));
        return {};
      }
      const auto x1f_count = std::ranges::count(value_string, '\x1f');
      values = std::vector<T>();
      values->reserve(x1f_count);
      for (auto it = value_string.begin(), se = value_string.end(); it < se;) {
        auto fr = std::ranges::find(it, se, '\x1f');
        if (fr == se) break;
        if (auto res = converter<T>{}(string_view<char>(it, fr)); !res) return res.error().relay();
        else values->push_back(std::move(*res));
        it = fr + 1;
      }
      return {};
    }
  };

  option(string<char> Key, const source_line& sl = here()) {
    if (const auto sp = create_slot<option>(sl); !sp) error(errors::slot_creation_failed).go_off(sl); // fatal
    else if (auto res = sp->init(std::move(Key)); !res) res.error().add_footprint().go_off(sl); // fatal
    else _id = sp->id;
  }

  template<typename Self> auto&& description(this Self&& self, string<char> Desc)
    requires(!is_const<remove_ref<Self>>) {
    const auto sp = self.get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out(); // warning
    else sp->description = std::move(Desc);
    return static_cast<Self&&>(self);
  }

  template<typename Self> auto&& required(this Self&& self, bool Required = true)
    requires(!is_const<remove_ref<Self>>) {
    const auto sp = self.get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out(); // warning
    else sp->required = Required;
    return static_cast<Self&&>(self);
  }

  template<typename Self> auto&& multiple(this Self&& self, bool Multiple = true)
    requires(!is_const<remove_ref<Self>>) {
    const auto sp = self.get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out(); // warning
    else sp->multiple = Multiple;
    return static_cast<Self&&>(self);
  }

  template<typename Self> auto&& default_value(this Self&& self, T Value)
    requires(!is_const<remove_ref<Self>>) {
    const auto sp = self.get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off(); // fatal
    if (auto res = converter<T>{}(Value)) {
      sp->default_value_string = std::move(*res);
      sp->default_value = std::move(Value);
    } else res.error().add_footprint().fizzle_out(sp->source_line); // warning
    return static_cast<Self&&>(self);
  }

  template<typename Self> auto&& alias(this Self&& self, string<char> Alias)
    requires(!is_const<remove_ref<Self>>) {
    const auto sp = self.get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off(); // fatal
    const auto [it, b] = name_map.emplace(Alias, self.id());
    if (!b) error(errors::operation_failed, format("Duplicate key: ", Alias)).fizzle_out(); // warning
    else if (const auto sp = self.get_slot(&self); sp) sp->aliases.push_back(std::move(Alias));
    return static_cast<Self&&>(self);
  }

  const auto& value() const {
    if (!internal::parsed) error(errors::invalid_operation, "value required before parsing").go_off(); // fatal
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off(); // fatal
    if (auto res = sp->update_value(); !res) res.error().add_footprint().go_off(sp->source_line); // fatal
    const auto& values = *sp->values;
    if (values.empty()) error(errors::missing_required_argument).go_off(sp->source_line); // fatal
    return values.front();
  }

  const auto& values() const {
    if (!internal::parsed) error(errors::invalid_operation, "values required before parsing").go_off(); // fatal
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off(); // fatal
    if (auto res = sp->update_value(); !res) res.error().add_footprint().go_off(sp->source_line); // fatal
    return *sp->values;
  }

  bool has_value() const {
    const auto sp = get_slot(this);
    return sp && (sp->specified || sp->default_value.has_value());
  }

  bool specified() const {
    const auto sp = get_slot(this);
    return sp && sp->specified;
  }
};

template<typename T> handle::option<T> option(string<char> Name, const source_line& sl = here()) {
  return handle::option<T>(std::move(Name), sl);
}

/// MARK: flag

class handle::flag : public handle {
public:
  struct slot : handle::slot {
    static const slot empty_slot;

    std::expected<void, error> init(string<char>&& Key) {
      const auto [it, b] = name_map.emplace(Key, id);
      if (!b) return std::unexpected(error(errors::operation_failed, format("Duplicate key: ", Key)));
      key = std::move(Key);
      type = handle::type::flag;
      return {};
    }
  };

  flag(string<char>&& Key, const source_line& sl = here()) {
    if (const auto sp = create_slot<flag>(sl); !sp) error(errors::slot_creation_failed).go_off(sl); // fatal
    else if (auto res = sp->init(std::move(Key)); !res) res.error().add_footprint().go_off(sl); // fatal
    else _id = sp->id;
  }

  template<typename Self> auto&& description(this Self&& self, string<char> Desc)
    requires(!is_const<remove_ref<Self>>) {
    const auto sp = self.get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out(); // warning
    else sp->description = std::move(Desc);
    return static_cast<Self&&>(self);
  }

  template<typename Self> auto&& alias(this Self&& self, string<char> Alias)
    requires(!is_const<remove_ref<Self>>) {
    const auto sp = self.get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off(); // fatal
    const auto [it, b] = name_map.emplace(Alias, self.id());
    if (!b) error(errors::operation_failed, format("Duplicate key: ", Alias)).fizzle_out(); // warning
    else sp->aliases.push_back(std::move(Alias));
    return static_cast<Self&&>(self);
  }

  bool value() const {
    if (!internal::parsed) error(errors::invalid_operation, "value required before parsing").go_off(); // fatal
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off(); // fatal
    return sp->specified;
  }

  bool has_value() const { return true; }

  bool specified() const {
    const auto sp = get_slot(this);
    return sp && sp->specified;
  }
};

inline handle::flag flag(string<char> Key, const source_line& sl = here()) {
  return handle::flag(std::move(Key), sl);
}

/// MARK: print_help
inline void print_help();

/// MARK: parse helper

namespace internal {

struct handle_access : handle {
  using handle::name_map;
  using handle::positionals;
};

inline void print_help() { argument::print_help(); }

inline std::vector<string<char>> collect_argv(int argc, char** argv, string<char>& name) {
  std::vector<string<char>> args;
#if defined(_WIN32) || defined(_WIN64)
  int c = 0;
  auto v = ::CommandLineToArgvW(::GetCommandLineW(), &c);
  if (!v) error(errors::operation_failed, "CommandLineToArgvW failed", ::GetLastError()).go_off(); // fatal
  args.reserve(static_cast<size_t>(c));
  for (int i = 0; i < c; ++i) args.emplace_back(unicode<char>(std::wstring_view(v[i])));
  ::LocalFree(v);
#else
  args.reserve(static_cast<size_t>(argc));
  for (int i = 0; i < argc; ++i) args.emplace_back(argv[i]);
#endif
  if (!args.empty() && name.empty()) name = unicode<char>(file::stem(args[0]));
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

inline handle::slot* get_argument_slot(general_slotid id) { return general_slot::get<handle>(id); }

inline handle::slot* find_named_slot(string_view<char> key) {
  const auto it = handle_access::name_map.find(string<char>(key));
  if (it == handle_access::name_map.end()) return nullptr;
  return get_argument_slot(it->second);
}

inline void append_option_value(handle::slot& slot, string_view<char> value) {
  slot.value_string += value;
  slot.value_string.push_back('\x1f');
}

inline void argument_error(ministr<char> msg) {
  print_help();
  error(errors::invalid_command_line_argument, string<char>(msg)).go_off(); // fatal
}
} // namespace internal

/// MARK: parse

inline void parse(int argc, char** argv) {
  if (internal::parsed) error(errors::invalid_operation, "Arguments have already been parsed").go_off(); // fatal
  auto args = internal::collect_argv(argc, argv, argument::program_name);
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
      if (key == "--help" || key == "-h") {
        internal::print_help();
        std::exit(0);
      }
      auto* sp = internal::find_named_slot(key);
      if (!sp) internal::argument_error(format("Unknown option: ", key));

      if (sp->type == handle::type::option) {
        if (!specified && i + 1 < args.size()) {
          const auto& next = args[i + 1];
          if (!internal::is_option_token(next) || internal::is_negative_number_token(next))
            value = next, specified = true, ++i;
        }
        if (!specified) internal::argument_error(format("Missing value for option: ", key));
        if (sp->specified && !sp->multiple)
          internal::argument_error(format("Option does not accept multiple values: ", key));
        sp->specified = true;
        internal::append_option_value(*sp, value);
      } else if (specified) internal::argument_error(format("Unexpected value for flag: ", key));
      else sp->specified = true;
      continue;
    }

    if (positional_index < internal::handle_access::positionals.size()) {
      auto* sp = internal::get_argument_slot(internal::handle_access::positionals[positional_index]);
      if (!sp) internal::argument_error(format("Unknown, positional argument: ", tok));
      sp->specified = true;
      sp->value_string = tok;
      ++positional_index;
    } else internal::argument_error(format("Unexpected positional argument: ", tok));
  }

  bool missing = false;
  string<char> missing_message = "Missing required options";
  std::vector<general_slotid> checked;
  checked.reserve(internal::handle_access::name_map.size() + internal::handle_access::positionals.size());

  auto check_required = [&](general_slotid id) {
    if (std::ranges::find(checked, id) != checked.end()) return;
    checked.push_back(id);
    const auto sp = internal::get_argument_slot(id);
    if (sp && sp->required && !sp->specified) {
      const auto label = sp->type == handle::type::positional ? sp->metavar : sp->key;
      if (!missing) {
        missing = true;
        missing_message += format(": ", label);
      } else missing_message += format(", ", label);
    }
  };

  for (const auto& [name, id] : internal::handle_access::name_map) check_required(id);
  for (const auto id : internal::handle_access::positionals) check_required(id);

  if (missing) internal::argument_error(missing_message);
  internal::parsed = true;
}

template<typename T> inline const handle::positional<T>::slot handle::positional<T>::slot::empty_slot{};
template<typename T> inline const handle::option<T>::slot handle::option<T>::slot::empty_slot{};
inline const handle::flag::slot handle::flag::slot::empty_slot{};

inline void print_help() {
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
    if (slot.type == handle::type::option) return default_option_metavar(slot.key);
    if (slot.type == handle::type::positional) return string<char>("ARG");
    return string<char>{};
  };

  auto append_note = [](string<char>& rhs, string_view<char> note) {
    if (!rhs.empty()) rhs += " ";
    rhs += note;
  };

  auto detail_text = [&](const handle::slot& slot) {
    string<char> rhs = slot.description;
    if (slot.required) append_note(rhs, "(required)");
    if (slot.multiple) append_note(rhs, "(multiple)");
    if (!slot.default_value_string.empty()) append_note(rhs, format("(default: ", slot.default_value_string, ")"));
    return rhs;
  };

  auto pad_right = [](string_view<char> text, size_t width) {
    string<char> out(text);
    if (out.size() < width) out.append(string(width - out.size(), ' '));
    return out;
  };

  auto print_entry = [&](string_view<char> lhs, string_view<char> rhs) {
    if (rhs.empty()) print("  ", lhs);
    else print("  ", pad_right(lhs, 24), " ", rhs);
  };

  std::vector<general_slotid> option_ids;
  option_ids.reserve(internal::handle_access::name_map.size());
  for (const auto& [name, id] : internal::handle_access::name_map) {
    const auto sp = internal::get_argument_slot(id);
    if (!sp || sp->type == handle::type::positional || sp->key != name) continue;
    if (std::ranges::find(option_ids, id) == option_ids.end()) option_ids.push_back(id);
  }
  std::ranges::sort(option_ids, [](general_slotid a, general_slotid b) {
    if (a.index != b.index) return a.index < b.index;
    return a.generation < b.generation;
  });

  if (!program_name.empty()) print(program_name);
  if (!description.empty()) print(description);
  if (!program_name.empty() || !description.empty()) print("");

  string<char> usage = format("Usage: ", program_name.empty() ? "app" : program_name);
  for (const auto id : option_ids) {
    const auto sp = internal::get_argument_slot(id);
    if (!sp) continue;
    string<char> item = sp->key;
    if (sp->type == handle::type::option) item += format(" ", display_metavar(*sp));
    if (sp->multiple) item = format("[", item, "]...");
    else if (!sp->required) item = format("[", item, "]");
    usage += format(" ", item);
  }
  for (const auto id : internal::handle_access::positionals) {
    const auto sp = internal::get_argument_slot(id);
    if (!sp) continue;
    const auto mv = display_metavar(*sp);
    if (sp->required) usage += format(" ", mv);
    else usage += format(" [", mv, "]");
  }
  print(usage);

  bool has_value_options = false;
  for (const auto id : option_ids) {
    const auto sp = internal::get_argument_slot(id);
    if (sp && sp->type == handle::type::option) {
      has_value_options = true;
      break;
    }
  }
  if (has_value_options) print("Note: Options with values may be written as either --key VALUE or --key=VALUE.");

  print("");
  print("Options:");
  print_entry("--help, -h", "Show this help message and exit");
  for (const auto id : option_ids) {
    const auto sp = internal::get_argument_slot(id);
    if (!sp) continue;
    string<char> lhs = sp->key;
    for (const auto& alias : sp->aliases) lhs += format(", ", alias);
    // if (sp->type == handle::type::option) lhs += format(" ", display_metavar(*sp));
    print_entry(lhs, detail_text(*sp));
  }

  if (!internal::handle_access::positionals.empty()) {
    print("");
    print("Positional arguments:");
    for (const auto id : internal::handle_access::positionals) {
      const auto sp = internal::get_argument_slot(id);
      if (!sp) continue;
      print_entry(display_metavar(*sp), detail_text(*sp));
    }
  }

  if (!epilog.empty()) {
    print("");
    print(epilog);
  }
}
} // namespace yw::argument
