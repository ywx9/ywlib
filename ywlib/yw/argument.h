#pragma once
#include "yw/error.h"
#include "yw/slotset.h"
#include "yw/tuple.h"

#ifdef _WIN32
#include <shellapi.h>
#endif

/**
 * \note コマンドライン引数について
 * Windows では GetCommandLineW + CommandLineToArgvW を使用する。
 * これは、環境の影響を最小限に抑えて UTF-8 に変換するためである。
 *
 * \note コマンドライン引数に関するエラー処理について
 * 以下の3種に分別して考える
 *  - 定義エラー: エラー情報を表示してfatal_errorで終了させる。
 *  - 引数エラー: --helpを表示した上でfatal_errorで終了させる。
 *  - 使用エラー: エラー情報を表示してfatal_errorで終了させる。
 */

namespace yw::errors {
define_error(argument_invalid_definition);
define_error(argument_invalid_argument);
define_error(argument_invalid_usage);
} // namespace yw::errors

namespace yw::argument {

/// name of application used in help text. This is overwritten in parsing.
inline std::string name;

/// description of application used in help text
inline std::string description;

/// epilog (ex: example usage) of application used in help text
inline std::string epilog;

//////////////////////////////////////// MARK: converter

template<typename T> struct converter {
  static std::expected<T, error_trace> operator()(std::string_view) {
    return unexpected_error(errors::argument_invalid_argument, "Invalid value for argument");
  }
};

template<> struct converter<std::string> {
  static std::expected<std::string, error_trace> operator()(std::string_view value) { return std::string(value); }
};

template<> struct converter<bool> {
  static std::expected<bool, error_trace> operator()(std::string_view value) {
    if (value == "1" || value == "true" || value == "on" || value == "yes") return true;
    if (value == "0" || value == "false" || value == "off" || value == "no") return false;
    return unexpected_error(errors::argument_invalid_argument, format("Invalid bool: {}", value));
  }
  static std::expected<std::string, error_trace> operator()(bool value) { return value ? "true" : "false"; }
};

template<typename T> requires(integral<T> && !is_bool<T>) struct converter<T> {
  static std::expected<T, error_trace> operator()(std::string_view value) {
    T out{};
    const auto* b = value.data();
    const auto* e = b + value.size();
    const auto r = std::from_chars(b, e, out);
    if (r.ec != std::errc() || r.ptr != e)
      return unexpected_error(errors::argument_invalid_argument, format("Invalid integer: {}", value));
    return out;
  }
  static std::expected<std::string, error_trace> operator()(T value) { return std::to_string(value); }
};

template<typename T> requires(float_type<T>) struct converter<T> {
  static std::expected<T, error_trace> operator()(std::string_view value) {
    std::string s(value);
    char* end = nullptr;
    errno = 0;
    const auto v = std::strtold(s.c_str(), &end);
    if (errno != 0 || end != s.c_str() + s.size())
      return unexpected_error(errors::argument_invalid_argument, format("Invalid number: {}", value));
    return static_cast<T>(v);
  }
  static std::expected<std::string, error_trace> operator()(T value) { return std::to_string(value); }
};

//////////////////////////////////////// MARK: handle

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
  std::string key;
  std::string metavar;
  std::string description;
  handle::type type;
  bool required = false;
  bool multiple = false;
  bool specified = false;
  virtual ~slot() = default;
  virtual std::expected<void, error_trace> parse_value(std::string_view) {
    return unexpected_error(errors::argument_invalid_argument, "Invalid value for argument");
  }
  virtual std::optional<std::string> default_value_string() const { return std::nullopt; }
};

namespace internal {
inline static bool parsed = false;
inline static slotset<handle::slot> slots;
inline static std::unordered_map<std::string, handle::slotid> name_map;
inline static std::vector<handle::slotid> positionals;
} // namespace internal

//////////////////////////////////////// MARK: positional

template<typename T> class handle::positional : public handle {
  positional(slotid Id) : handle(Id) {}

public:
  struct slot : handle::slot {
    std::optional<T> value;
    std::optional<T> default_value;

    static std::expected<positional, error_trace> create(std::string Metavar) {
      const auto id = internal::slots.add(std::make_unique<slot>());
      const auto sp = dynamic_cast<slot*>(internal::slots.get(id));
      if (!sp) return unexpected_error(errors::operation_failed, "Failed to create positional slot");
      sp->id = id;
      sp->key = uint_to_string(internal::positionals.size());
      sp->metavar = std::move(Metavar);
      sp->type = handle::type::positional;
      internal::positionals.push_back(id);
      return positional(id);
    }

    static slot& get(slotid Id) {
      const auto sp = dynamic_cast<slot*>(internal::slots.get(Id));
      if (!sp) fatal_error(errors::operation_failed, "Failed to get positional slot");
      return *sp;
    }

    std::expected<void, error_trace> parse_value(std::string_view Value) override {
      if (auto res = converter<T>{}(Value); !res) {
        yw::error& e = res.error().error;
        e.message = format("{} at #{}", e.message, key);
        return unexpected_error(res.error());
      } else value = std::move(*res);
      specified = true;
      return {};
    }

    std::optional<std::string> default_value_string() const override {
      if (default_value.has_value()) {
        if (auto res = converter<T>{}(*default_value); res) return *res;
        else return std::nullopt;
      } else return std::nullopt;
    }
  };

  auto& description(std::string Desc) { return slot::get(id).description = std::move(Desc), *this; }
  auto& required(bool Required) { return slot::get(id).required = Required, *this; }
  auto& default_value(T Value) { return slot::get(id).default_value = std::move(Value), *this; }

  const auto& value() const {
    if (!internal::parsed) fatal_error(errors::argument_invalid_usage, "Call before parsing");
    auto& s = slot::get(id);
    if (s.specified) return *s.value;
    if (!s.default_value.has_value()) fatal_error(errors::argument_invalid_usage, "No value available");
    return *s.default_value;
  }

  bool has_value() const {
    if (!internal::parsed) fatal_error(errors::argument_invalid_usage, "Call before parsing");
    auto& s = slot::get(id);
    return s.specified || s.default_value.has_value();
  }

  bool specified() const {
    if (!internal::parsed) fatal_error(errors::argument_invalid_usage, "Call before parsing");
    return slot::get(id).specified;
  }
};

template<typename T> handle::positional<T> positional(std::string Metavar) {
  auto res = handle::positional<T>::slot::create(std::move(Metavar));
  if (!res) fatal_error(res.error());
  return std::move(*res);
}

//////////////////////////////////////// MARK: option

template<typename T> class handle::option : public handle {
  option(slotid Id) : handle(Id) {}

public:
  struct slot : handle::slot {
    std::vector<T> values;
    std::optional<T> default_value;

    static std::expected<option, error_trace> create(std::string Key) {
      const auto id = internal::slots.add(std::make_unique<slot>());
      const auto [it, b] = internal::name_map.emplace(Key, id);
      if (!b) return unexpected_error(errors::argument_invalid_definition, format("Duplicate key: {}", Key));
      const auto sp = dynamic_cast<slot*>(internal::slots.get(id));
      if (!sp) return unexpected_error(errors::operation_failed, "Failed to create option slot");
      sp->id = id;
      sp->key = std::move(Key);
      sp->type = handle::type::option;
      return option(id);
    }

    static slot& get(slotid Id) {
      const auto sp = dynamic_cast<slot*>(internal::slots.get(Id));
      if (!sp) fatal_error(errors::operation_failed, "Failed to get option slot");
      return *sp;
    }

    std::expected<void, error_trace> parse_value(std::string_view Value) override {
      if (auto res = converter<T>{}(Value); !res) {
        yw::error& e = res.error().error;
        e.message = format("{} as '{}'", e.message, key);
        return unexpected_error(res.error());
      } else if (multiple || values.empty()) values.push_back(std::move(*res));
      specified = true;
      return {};
    }

    std::optional<std::string> default_value_string() const override {
      if (default_value.has_value()) {
        if (auto res = converter<T>{}(*default_value); res) return *res;
        else return std::nullopt;
      } else return std::nullopt;
    }
  };

  auto& metavar(std::string Metavar) { return slot::get(id).metavar = std::move(Metavar), *this; }
  auto& description(std::string Desc) { return slot::get(id).description = std::move(Desc), *this; }
  auto& required(bool Required) { return slot::get(id).required = Required, *this; }
  auto& multiple(bool Multiple) { return slot::get(id).multiple = Multiple, *this; }
  auto& default_value(T Value) { return slot::get(id).default_value = std::move(Value), *this; }

  auto& alias(std::string Alias) {
    const auto [it, b] = internal::name_map.emplace(Alias, id);
    if (!b) fatal_error(errors::argument_invalid_definition, format("Duplicate key: {}", Alias));
    return *this;
  }

  const auto& value() const {
    if (!internal::parsed) fatal_error(errors::argument_invalid_usage, "Call before parsing");
    auto& s = slot::get(id);
    if (s.specified) return s.values.front();
    if (!s.default_value.has_value()) fatal_error(errors::argument_invalid_usage, "No value available");
    return *s.default_value;
  }

  const auto& values() const {
    if (!internal::parsed) fatal_error(errors::argument_invalid_usage, "Call before parsing");
    return slot::get(id).values;
  }

  bool has_value() const {
    if (!internal::parsed) fatal_error(errors::argument_invalid_usage, "Call before parsing");
    const auto& s = slot::get(id);
    return s.specified || s.default_value.has_value();
  }

  bool specified() const {
    if (!internal::parsed) fatal_error(errors::argument_invalid_usage, "Call before parsing");
    return slot::get(id).specified;
  }
};

template<typename T> handle::option<T> option(std::string Name) {
  auto res = handle::option<T>::slot::create(std::move(Name));
  if (!res) fatal_error(res.error());
  return std::move(*res);
}

//////////////////////////////////////// MARK: flag

class handle::flag : public handle {
  flag(slotid Id) : handle(Id) {}

public:
  struct slot : handle::slot {
    static std::expected<flag, error_trace> create(std::string Key) {
      const auto id = internal::slots.add(std::make_unique<slot>());
      const auto [it, b] = internal::name_map.emplace(Key, id);
      if (!b) return unexpected_error(errors::argument_invalid_definition, format("Duplicate key: {}", Key));
      const auto sp = dynamic_cast<slot*>(internal::slots.get(id));
      if (!sp) return unexpected_error(errors::operation_failed, "Failed to create flag slot");
      sp->id = id;
      sp->key = std::move(Key);
      sp->type = handle::type::flag;
      return flag(id);
    }

    static slot& get(slotid Id) {
      const auto sp = dynamic_cast<slot*>(internal::slots.get(Id));
      if (!sp) fatal_error(errors::operation_failed, "Failed to get flag slot");
      return *sp;
    }

    std::expected<void, error_trace> parse_value(std::string_view value) override { return {}; }
  };

  auto& description(std::string Desc) { return slot::get(id).description = std::move(Desc), *this; }

  auto& alias(std::string Alias) {
    const auto [it, b] = internal::name_map.emplace(Alias, id);
    if (!b) fatal_error(errors::argument_invalid_definition, format("Duplicate key: {}", Alias));
    return *this;
  }

  bool value() const {
    if (!internal::parsed) fatal_error(errors::argument_invalid_usage, "Call before parsing");
    return slot::get(id).specified;
  }

  bool has_value() const {
    if (!internal::parsed) fatal_error(errors::argument_invalid_usage, "Call before parsing");
    return true;
  }

  bool specified() const { return value(); }
};

inline handle::flag flag(std::string Name) {
  auto res = handle::flag::slot::create(std::move(Name));
  if (!res) fatal_error(res.error());
  return std::move(*res);
}

//////////////////////////////////////// MARK: print_help

namespace internal {
inline std::string default_option_metavar(std::string_view key) {
  while (!key.empty() && key.front() == '-') key.remove_prefix(1);
  std::string out;
  out.reserve(key.size());
  for (const char c : key) {
    if (c == '-') out.push_back('_');
    else out.push_back(char(std::toupper(static_cast<unsigned char>(c))));
  }
  return out.empty() ? "VALUE" : out;
}
} // namespace internal

inline void print_help() {
  const std::string app_name = name.empty() ? "app" : name;

  auto default_option_metavar = [](std::string_view key) {
    while (!key.empty() && key.front() == '-') key.remove_prefix(1);

    std::string out;
    out.reserve(key.size());

    for (const char c : key) {
      if (c == '-') out.push_back('_');
      else out.push_back(char(std::toupper(static_cast<unsigned char>(c))));
    }

    return out.empty() ? std::string("VALUE") : out;
  };

  auto display_metavar = [&](const handle::slot& slot) {
    if (!slot.metavar.empty()) return slot.metavar;

    if (slot.type == handle::type::option) { return default_option_metavar(slot.key); }

    if (slot.type == handle::type::positional) { return std::string("ARG"); }

    return std::string{};
  };

  auto names_of = [](const handle::slot& slot) {
    std::vector<std::string> names;
    names.push_back(slot.key);
    std::vector<std::string> aliases;
    for (const auto& [name, id] : internal::name_map)
      if (id == slot.id && name != slot.key) aliases.push_back(name);
    std::ranges::sort(aliases);
    names.insert(names.end(), aliases.begin(), aliases.end());
    return names;
  };

  auto primary_usage_name = [](const handle::slot& slot) { return slot.key; };

  auto option_usage = [&](const handle::slot& slot) {
    std::string s = primary_usage_name(slot);
    if (slot.type == handle::type::option) {
      s += " ";
      s += display_metavar(slot);
    }
    if (slot.multiple) s = "[" + s + "]...";
    else if (!slot.required) s = "[" + s + "]";
    return s;
  };

  auto option_lhs = [&](const handle::slot& slot) {
    const auto names = names_of(slot);
    std::string lhs;
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

  auto append_note = [](std::string& rhs, std::string_view note) {
    if (!rhs.empty()) rhs += " ";
    rhs += note;
  };

  auto detail_text = [&](const handle::slot& slot) {
    std::string rhs;
    if (!slot.description.empty()) { rhs += slot.description; }
    if (slot.required) { append_note(rhs, "(required)"); }
    if (slot.multiple) { append_note(rhs, "(multiple)"); }
    if (auto dv = slot.default_value_string(); dv.has_value()) append_note(rhs, format("(default: {})", *dv));
    return rhs;
  };

  auto print_entry = [](std::string_view lhs, std::string_view rhs) {
    if (rhs.empty()) print("  {}", lhs);
    else print("  {:<24} {}", lhs, rhs);
  };

  if (!name.empty()) print("{}", name);
  if (!description.empty()) print("{}", description);
  if (!name.empty() || !description.empty()) print("");

  std::string usage = format("Usage: {}", app_name);

  for (const auto& slot : internal::slots) {
    if (slot.type == handle::type::positional) continue;
    usage += " ";
    usage += option_usage(slot);
  }

  for (const auto pid : internal::positionals) {
    const auto* p = internal::slots.get(pid);
    if (!p) continue;

    const auto mv = display_metavar(*p);

    if (p->required) usage += format(" {}", mv);
    else usage += format(" [{}]", mv);
  }

  print("{}", usage);

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

//////////////////////////////////////// MARK: parse helper

namespace internal {

inline std::expected<std::vector<std::string>, error_trace> collect_argv(
  int argc, char** argv, std::string& program_name) {
  std::vector<std::string> args;
#if defined(_WIN32) || defined(_WIN64)
  int c = 0;
  auto v = ::CommandLineToArgvW(::GetCommandLineW(), &c);
  if (!v) return unexpected_error(errors::operation_failed, "CommandLineToArgvW failed", int(::GetLastError()));
  args.reserve(static_cast<size_t>(c));
  for (int i = 0; i < c; ++i) args.emplace_back(unicode<char>(std::wstring_view(v[i])));
  ::LocalFree(v);
#else
  args.reserve(static_cast<size_t>(argc));
  for (int i = 0; i < argc; ++i) args.emplace_back(argv[i]);
#endif
  if (!args.empty() && name.empty()) program_name = unicode<char>(std::filesystem::path(args.front()).stem().native());
  return args;
}

inline bool is_option_token(const std::string_view tok) { return tok.size() > 1 && tok[0] == '-'; }

inline std::optional<std::pair<std::string_view, std::string_view>> split_eq(std::string_view tok) {
  if (auto i = tok.find('='); i == std::string_view::npos) return std::nullopt;
  else if (i == 0) return std::nullopt;
  else return std::pair<std::string_view, std::string_view>{tok.substr(0, i), tok.substr(i + 1)};
}

inline bool is_negative_number_token(std::string_view tok) {
  return !(tok.size() < 2 || tok[0] != '-') && (is_digit(tok[1]) || tok[1] == '.');
}

inline void argument_error(null_terminated<char> msg) {
  print_help();
  fatal_error(errors::argument_invalid_argument, msg);
}
} // namespace internal

//////////////////////////////////////// MARK: parse

inline void parse(int argc, char** argv) {
  //-- 初実行の確認
  if (internal::parsed) fatal_error(errors::argument_invalid_usage, "Arguments have already been parsed");
  //-- 引数解釈
  auto args = internal::collect_argv(argc, argv, argument::name);
  if (!args) fatal_error(args.error());

  bool after_double_dash = false; // "--"以降の引数はすべてpositionalとして扱う
  size_t positional_index = 0;    // positionalの数を数える

  for (size_t i = 1; i < args->size(); ++i) {
    const std::string_view tok = (*args)[i];
    if (!after_double_dash && tok == "--") { // 最初の"--"を検出
      after_double_dash = true;
      continue;
    }
    if (!after_double_dash && internal::is_option_token(tok)) { // optionらしいものを発見
      //-- いったんtoken全体をkey値としておく(--key value想定)
      std::string_view key = tok;
      std::string_view value{};
      bool specified = false;
      //-- token内に"="が含まれているなら、key=valueに分割
      if (auto kv = internal::split_eq(tok)) {
        key = kv->first;
        value = kv->second;
        specified = true;
      }
      //-- 宣言されたkeyか確認
      auto it = internal::name_map.find(std::string(key));
      if (it == internal::name_map.end()) {
        if (key == "--help" || key == "-h") {
          print_help();
          std::exit(0);
        }
        internal::argument_error(format("Unknown option: {}", key));
      }
      const auto sp = internal::slots.get(it->second);
      if (!sp) internal::argument_error(format("Unknown option: {}", key));
      //-- optionの場合はvalueが必須
      if (sp->type == handle::type::option) {
        //-- --key=valueでないなら次を確認
        if (!specified && i + 1 < args->size()) {
          const auto next = (*args)[i + 1];
          if (!internal::is_option_token(next) || internal::is_negative_number_token(next))
            value = next, specified = true, ++i;
        }
        if (!specified) internal::argument_error(format("Missing value for option: {}", key));
        if (auto res = sp->parse_value(value); !res) print_help(), fatal_error(res.error());
      } else if (specified) //-- flagの場合はvalueがあってはならない
        internal::argument_error(format("Unexpected value for flag: {}", key));
      else sp->specified = true; //-- flagがあったことはspecified==trueで表現する
      continue;
    }
    //-- positionalの場合
    if (positional_index < internal::positionals.size()) {
      const auto sp = internal::slots.get(internal::positionals[positional_index]);
      if (!sp) internal::argument_error(format("Unknown, positional argument: {}", tok));
      if (auto res = sp->parse_value(tok); !res) fatal_error(res.error());
      ++positional_index;
    } else internal::argument_error(format("Unexpected positional argument: {}", tok));
  }
  //-- requiredのチェック
  bool missing = false;
  std::string missing_message = "Missing required options";
  for (const auto& slot : internal::slots) {
    if (slot.required && !slot.specified) {
      if (!missing) {
        missing = true;
        missing_message += format(": {}", slot.key);
      } else missing_message += format(", {}", slot.key);
    }
  }
  if (missing) internal::argument_error(missing_message);
  internal::parsed = true;
}
} // namespace yw::argument
