#pragma once
#include "yw/error.h"
#include "yw/slotset.h"
#include "yw/tuple.h"

#ifdef _WIN32
#include <shellapi.h>
#endif

/**
 * \note
 * On Windows, command-line arguments are obtained from the OS
 * (GetCommandLineW + CommandLineToArgvW) and converted to UTF-8.
 * The C runtime's argc/argv are not used as the authoritative source.
 */

namespace yw::errors {
define_error(argument_invalid_declaration);
define_error(argument_invalid_usage);
} // namespace yw::errors

// namespace yw::test {

// inline class {
// public:
//   class handle {
//   public:
//     struct slot {
//       std::string name;
//       std::string metavar;
//       std::string description;
//       std::vector<std::string> aliases;
//       bool required = false;
//       bool multiple = false;
//       bool has_value = false;
//       bool specified = false;
//       bool is_option = false;
//     };
//     using slotid = typename slotset<slot>::slotid;

//   protected:
//     slotid id;
//     source pos;

//     handle(slotid id, const source& s) : id(id), pos(s) {}
//     handle(const handle&) = delete;
//     handle(handle&& other) noexcept : id(std::exchange(other.id, {})), pos(other.pos) {}

//     template<derived_from<handle> Handle, int Type> static Handle initialize(std::string Name, const source& _);
//     template<derived_from<handle> Handle> Handle& metavar(std::string Metavar);
//     template<derived_from<handle> Handle> Handle& description(std::string Description);
//     template<derived_from<handle> Handle> Handle& alias(std::string Alias);
//     template<derived_from<handle> Handle> Handle& required(bool Required);
//     template<derived_from<handle> Handle> Handle& multiple(bool Multiple);
//     template<derived_from<handle> Handle, typename T> Handle& default_value(T DefaultValue);
//     bool has_value() const;
//     bool specified() const;
//     static std::unexpected<error_trace> error(decltype(errors::success) e, const source& _) {
//       return unexpected_error(e, {}, 0, npos, _);
//     }
//   };

//   class flag_handle;
//   friend class flag_handle;

//   template<typename T> class option_handle;
//   template<typename T> friend class option_handle;

//   template<typename T> class positional_handle;
//   template<typename T> friend class positional_handle;

// private:
//   slotset<handle::slot> slots;
//   std::unordered_map<std::string, handle::slotid> name_map;
//   std::string _name;
//   unsigned _positional_count = 0;
//   bool _parsed = false;

// public:
//   std::string description;
//   std::string epilog;

//   flag_handle flag(std::string Name, const source& _ = {});
//   template<typename T> option_handle<T> option(std::string Name, const source& _ = {});
//   template<typename T> positional_handle<T> positional(const source& _ = {});

//   std::expected<void, error_trace> parse(int argc, char** argv);
// } argument;

// //////////////////////////////////////// MARK: argument.slots

// template<derived_from<decltype(argument)::handle> Handle, int Type>
// Handle decltype(argument)::handle::initialize(std::string Name, const source& _) {
//   if constexpr (Type) {
//     if (Name.empty() || Name == "-" || Name == "--" || !Name.starts_with("-"))
//       throw handle::error(errors::argument_invalid_option_name, _);
//   }
//   const auto id = argument.slots.add(std::make_unique<typename Handle::slot>());
//   const auto sp = static_cast<typename Handle::slot*>(argument.slots.get(id));
//   if (!sp) throw handle::error(errors::operation_failed, _);
//   sp->name = std::move(Name);
//   sp->is_option = Type == 2;
//   const auto [it, b] = argument.name_map.emplace(sp->name, id);
//   if (!b) throw handle::error(errors::argument_duplicate_option, _);
//   return Handle(id, _);
// }

// template<derived_from<decltype(argument)::handle> Handle>
// Handle& decltype(argument)::handle::metavar(std::string Metavar) {
//   const auto sp = static_cast<typename Handle::slot*>(argument.slots.get(id));
//   if (!sp) throw handle::error(errors::argument_unknown_option, pos);
//   sp->metavar = std::move(Metavar);
//   return static_cast<Handle&>(*this);
// }

// template<derived_from<decltype(argument)::handle> Handle>
// Handle& decltype(argument)::handle::description(std::string Description) {
//   const auto sp = static_cast<typename Handle::slot*>(argument.slots.get(id));
//   if (!sp) throw handle::error(errors::argument_unknown_option, pos);
//   sp->description = std::move(Description);
//   return static_cast<Handle&>(*this);
// }

// template<derived_from<decltype(argument)::handle> Handle> Handle& decltype(argument)::handle::alias(std::string Alias) {
//   if (Alias.empty() || Alias == "-" || Alias == "--" || !Alias.starts_with("-"))
//     throw handle::error(errors::argument_invalid_option_name, pos);
//   const auto sp = static_cast<typename Handle::slot*>(argument.slots.get(id));
//   if (!sp) throw handle::error(errors::argument_unknown_option, pos);
//   sp->aliases.push_back(std::move(Alias));
//   const auto& alias_name = sp->aliases.back();
//   const auto [it, b] = argument.name_map.emplace(alias_name, id);
//   if (!b) throw handle::error(errors::argument_duplicate_option, pos);
//   return static_cast<Handle&>(*this);
// }

// template<derived_from<decltype(argument)::handle> Handle> Handle& decltype(argument)::handle::required(bool Required) {
//   const auto sp = static_cast<typename Handle::slot*>(argument.slots.get(id));
//   if (!sp) throw handle::error(errors::argument_unknown_option, pos);
//   sp->required = Required;
//   return static_cast<Handle&>(*this);
// }

// template<derived_from<decltype(argument)::handle> Handle> Handle& decltype(argument)::handle::multiple(bool Multiple) {
//   const auto sp = static_cast<typename Handle::slot*>(argument.slots.get(id));
//   if (!sp) throw handle::error(errors::argument_unknown_option, pos);
//   sp->multiple = Multiple;
//   return static_cast<Handle&>(*this);
// }

// template<derived_from<decltype(argument)::handle> Handle, typename T>
// Handle& decltype(argument)::handle::default_value(T DefaultValue) {
//   const auto sp = static_cast<typename Handle::slot*>(argument.slots.get(id));
//   if (!sp) throw handle::error(errors::argument_unknown_option, pos);
//   sp->default_value = std::move(DefaultValue);
//   sp->has_value = true;
//   return static_cast<Handle&>(*this);
// }

// inline bool decltype(argument)::handle::has_value() const {
//   const auto sp = static_cast<const slot*>(argument.slots.get(id));
//   if (!sp) throw handle::error(errors::argument_unknown_option, pos);
//   return sp->has_value;
// }

// inline bool decltype(argument)::handle::specified() const {
//   const auto sp = static_cast<const slot*>(argument.slots.get(id));
//   if (!sp) throw handle::error(errors::argument_unknown_option, pos);
//   return sp->specified;
// }

// //////////////////////////////////////// MARK: argument.flag

// class decltype(argument)::flag_handle : public decltype(argument)::handle {
//   friend decltype(argument);
//   flag_handle(slotid id, const source& _) : decltype(argument)::handle(id, _) {}

// public:
//   struct slot : public decltype(argument)::handle::slot {
//     bool value = false;
//   };

//   auto& description(std::string Description) { return handle::description<flag_handle>(std::move(Description)); }
//   auto& alias(std::string Alias) { return handle::alias<flag_handle>(std::move(Alias)); }
//   auto& short_name(char ShortName) { return alias(std::string({'-', ShortName})); }
//   bool specified() const { return handle::specified(); }
//   bool has_value() const { return true; }

//   bool value() const {
//     if (!argument._parsed) throw handle::error(errors::argument_not_parsed, pos);
//     const auto sp = static_cast<const slot*>(argument.slots.get(id));
//     if (!sp) throw handle::error(errors::argument_unknown_option, pos);
//     return sp->value;
//   }
// };

// inline decltype(argument)::flag_handle decltype(argument)::flag(std::string Name, const source& _) {
//   return handle::initialize<flag_handle, 1>(std::move(Name), _);
// }

// //////////////////////////////////////// MARK: argument.option

// template<typename T> class decltype(argument)::option_handle : public decltype(argument)::handle {
//   friend decltype(argument);
//   option_handle(slotid id, const source& _) : decltype(argument)::handle(id, _) {}

// public:
//   struct slot : public decltype(argument)::handle::slot {
//     std::optional<T> default_value;
//     std::vector<T> values;
//   };

//   auto& metavar(std::string Metavar) { return handle::metavar<option_handle<T>>(std::move(Metavar)); }
//   auto& description(std::string Description) { return handle::description<option_handle<T>>(std::move(Description)); }
//   auto& alias(std::string Alias) { return handle::alias<option_handle<T>>(std::move(Alias)); }
//   auto& short_name(char ShortName) { return alias(std::string({'-', ShortName})); }
//   auto& required(bool Required = true) { return handle::required<option_handle<T>>(Required); }
//   auto& multiple(bool Multiple = true) { return handle::multiple<option_handle<T>>(Multiple); }
//   auto& default_value(T DefaultValue) { return handle::default_value<option_handle<T>>(std::move(DefaultValue)); }

//   bool specified() const { return handle::specified(); }
//   bool has_value() const { return handle::has_value(); }

//   const T& value() const {
//     if (!argument._parsed) throw handle::error(errors::argument_not_parsed, pos);
//     const auto sp = static_cast<const slot*>(argument.slots.get(id));
//     if (!sp) throw handle::error(errors::argument_unknown_option, pos);
//     if (sp->specified) return sp->values.front();
//     if (sp->has_value) return *sp->default_value;
//     throw handle::error(errors::argument_missing_value, pos);
//   }

//   const auto& values() const {
//     if (!argument._parsed) throw handle::error(errors::argument_not_parsed, pos);
//     const auto sp = static_cast<const slot*>(argument.slots.get(id));
//     if (!sp) throw handle::error(errors::argument_unknown_option, pos);
//     return sp->values;
//   }
// };

// template<typename T>
// decltype(argument)::option_handle<T> decltype(argument)::option(std::string Name, const source& _) {
//   return handle::initialize<option_handle<T>, 2>(std::move(Name), _);
// }

// //////////////////////////////////////// MARK: argument.positional

// template<typename T> class decltype(argument)::positional_handle : public decltype(argument)::handle {
//   friend decltype(argument);
//   positional_handle(slotid id, const source& _) : decltype(argument)::handle(id, _) {}

// public:
//   struct slot : public decltype(argument)::handle::slot {
//     std::optional<T> default_value;
//     std::optional<T> value;
//   };

//   auto& metavar(std::string Metavar) { return handle::metavar<positional_handle>(std::move(Metavar)); }
//   auto& description(std::string Description) { return handle::description<positional_handle>(std::move(Description)); }
//   auto& required(bool Required = true) { return handle::required<positional_handle>(Required); }
//   auto& default_value(T DefaultValue) { return handle::default_value<positional_handle>(std::move(DefaultValue)); }

//   bool specified() const { return handle::specified(); }
//   bool has_value() const { return handle::has_value(); }

//   const T& value() const {
//     if (!argument._parsed) throw handle::error(errors::argument_not_parsed, pos);
//     const auto sp = static_cast<const slot*>(argument.slots.get(id));
//     if (!sp) throw handle::error(errors::argument_unknown_option, pos);
//     if (sp->specified) return *sp->value;
//     if (sp->has_value) return *sp->default_value;
//     throw handle::error(errors::argument_missing_value, pos);
//   }
// };

// template<typename T> decltype(argument)::positional_handle<T> decltype(argument)::positional(const source& _) {
//   return handle::initialize<positional_handle<T>, 0>(uint_to_string(_positional_count++), _);
// }

// //////////////////////////////////////// MARK: helper for parsing

// namespace internal {

// inline std::expected<std::vector<std::string>, error_trace> collect_argv(
//   int argc, char** argv, std::string& program_name) {
//   std::vector<std::string> args;
// #if defined(_WIN32) || defined(_WIN64)
//   int c = 0;
//   auto v = ::CommandLineToArgvW(::GetCommandLineW(), &c);
//   if (!v) return unexpected_error(errors::operation_failed, "CommandLineToArgvW failed", int(::GetLastError()));
//   args.reserve(static_cast<size_t>(c));
//   for (int i = 0; i < c; ++i) args.emplace_back(unicode<char>(std::wstring_view(v[i])));
//   ::LocalFree(v);
// #else
//   args.reserve(static_cast<size_t>(argc));
//   for (int i = 0; i < argc; ++i) args.emplace_back(argv[i]);
// #endif
//   if (!args.empty()) program_name = unicode<char>(std::filesystem::path(args.front()).stem().native());
//   return args;
// }

// inline bool is_option_token(const std::string_view tok) { return tok.size() > 1 && tok[0] == '-'; }

// inline std::optional<tuple<std::string_view, std::string_view>> split_eq(std::string_view tok) {
//   if (auto i = tok.find('='); i == std::string_view::npos) return std::nullopt;
//   else if (i == 0) return std::nullopt;
//   else return tuple<std::string_view, std::string_view>{tok.substr(0, i), tok.substr(i + 1)};
// }

// inline bool is_negative_number_token(std::string_view tok) {
//   if (tok.size() < 2 || tok[0] != '-') return false;
//   return is_digit(tok[1]) || tok[1] == '.';
// }
// } // namespace internal

// //////////////////////////////////////// MARK: argument.parse

// inline std::expected<void, error_trace> decltype(argument)::parse(int argc, char** argv) {
//   if (argument._parsed) return unexpected_error(errors::argument_already_parsed, {});
//   auto args = internal::collect_argv(argc, argv, argument._name);
//   if (!args) return unexpected_error(args.error());
//   bool after_double_dash = false;
//   size_t positional_index = 0;
//   for (size_t i = 1; i < args->size(); ++i) {
//     const std::string_view tok = (*args)[i];
//     if (!after_double_dash && tok == "--") {
//       after_double_dash = true;
//       continue;
//     }
//     if (!after_double_dash && internal::is_option_token(tok)) {
//       std::string_view name = tok, value{};
//       bool specified = false;
//       if (auto kv = internal::split_eq(tok)) {
//         name = kv->first;
//         value = kv->second;
//         specified = true;
//       }
//       auto it = argument.name_map.find(std::string(name));
//       if (it == argument.name_map.end())
//         return unexpected_error(errors::argument_unknown_option, format("Unknown option: {}", name));
//       const auto sp = argument.slots.get(it->second);
//       if (!sp) return unexpected_error(errors::argument_unknown_option, format("Unknown option: {}", name));
//       if (sp->is_option) {
//         if (!specified && i + 1 < args->size()) {
//           const auto next = (*args)[i + 1];
//           if (!detail::is_option_token(next) || detail::is_negative_number_token(next)) {
//             value = next;
//             specified = true;
//             ++i;
//           }
//         }
//         if (!specified) return unexpected_error(errors::argument_missing_value, name);
//         sp->specified = true;
//       } else if (specified) return unexpected_error(errors::argument_unexpected_value, name);
//       continue;
//     }
//   }
// }
// } // namespace yw::test

namespace yw::argument {

/// used in help text
inline std::string description, epilog;

template<typename T> struct converter {
  static std::expected<T, error_trace> convert(std::string_view) {
    return unexpected_error(errors::argument_invalid_usage, "Invalid value for argument");
  }
};

template<> struct converter<std::string> {
  static std::expected<std::string, error_trace> convert(std::string_view value) { return std::string(value); }
};

template<> struct converter<bool> {
  static std::expected<bool, error_trace> convert(std::string_view value) {
    if (value == "1" || value == "true" || value == "on" || value == "yes") return true;
    if (value == "0" || value == "false" || value == "off" || value == "no") return false;
    return unexpected_error(errors::argument_invalid_usage, format("Invalid bool: {}", value));
  }
};

template<typename T> requires(integral<T> && !is_bool<T>) struct converter<T> {
  static std::expected<T, error_trace> convert(std::string_view value) {
    T out{};
    const auto* b = value.data();
    const auto* e = b + value.size();
    const auto r = std::from_chars(b, e, out);
    if (r.ec != std::errc() || r.ptr != e)
      return unexpected_error(errors::argument_invalid_usage, format("Invalid integer: {}", value));
    return out;
  }
};

template<typename T> requires(float_type<T>) struct converter<T> {
  static std::expected<T, error_trace> convert(std::string_view value) {
    std::string s(value);
    char* end = nullptr;
    errno = 0;
    const auto v = std::strtold(s.c_str(), &end);
    if (errno != 0 || end != s.c_str() + s.size())
      return unexpected_error(errors::argument_invalid_usage, format("Invalid number: {}", value));
    return static_cast<T>(v);
  }
};

class handle {
public:
  enum class type : uint8_t { positional, option, flag };

  struct slot {
    std::string name;
    std::string metavar;
    std::string description;
    std::vector<std::string> aliases;
    handle::type type;
    bool required = false;
    bool multiple = false;
    bool has_value = false;
  };

  using slotid = slotset<slot>::slotid;

protected:
  inline static bool _parsed = false;
  inline static slotset<slot> _slots;
  inline static std::vector<slotid> _positionals;

  slotid id;
  yw::source source; // エラー時に宣言位置を示すため

  handle(slotid Id, yw::source Src) : id(Id), source(Src) {}

  template<typename Mp> void set_helper(Mp Member, member_type<Mp> Value) {
    if (const auto sp = _slots.get(id)) sp->*Member = std::move(Value);
  }

  void metavar(std::string Metavar) { set_helper(&slot::metavar, std::move(Metavar)); }
  void description(std::string Desc) { set_helper(&slot::description, std::move(Desc)); }
  void alias(std::string Alias) { set_helper(&slot::aliases, std::vector<std::string>{std::move(Alias)}); }
  void required(bool Required) { set_helper(&slot::required, Required); }
  void multiple(bool Multiple) { set_helper(&slot::multiple, Multiple); }

public:
  template<typename T> class positional;
  template<typename T> class option;
  class flag;
};

template<typename T> class handle::positional : public handle {
public:
  struct slot : handle::slot {
    std::optional<T> value;
    std::optional<T> default_value;
  };

  auto& description(std::string Desc) { return handle::description(Desc), *this; }
  auto& required(bool Required) { return handle::required(Required), *this; }

  const auto& value() const {
    const auto sp = static_cast<slot*>(handle::_slots.get(id));
    if (sp && _parsed) {
      if (sp->value.has_value()) return *sp->value;
      if (sp->default_value.has_value()) return *sp->default_value;
    }
    throw unexpected_error(errors::argument_invalid_usage, "No value available");
  }
};

template<typename T> class handle::option : public handle {
public:
  struct slot : handle::slot {
    std::vector<T> values;
    std::optional<T> default_value;
  };

  auto& metavar(std::string Metavar) { return handle::metavar(Metavar), *this; }
  auto& description(std::string Desc) { return handle::description(Desc), *this; }
  auto& alias(std::string Alias) { return handle::alias(Alias), *this; }
  auto& required(bool Required) { return handle::required(Required), *this; }
  auto& multiple(bool Multiple) { return handle::multiple(Multiple), *this; }

  const auto& value() const {
    const auto sp = static_cast<slot*>(handle::_slots.get(id));
    if (sp && _parsed) {
      if (!sp->values.empty()) return sp->values.front();
      if (sp->default_value.has_value()) return *sp->default_value;
    }
    throw unexpected_error(errors::argument_invalid_usage, "No value available");
  }

  const auto& values() const {
    const auto sp = static_cast<slot*>(handle::_slots.get(id));
    if (sp && _parsed) return sp->values;
    throw unexpected_error(errors::argument_invalid_usage, "No values available");
  }
};

class handle::flag : public handle {
public:
  using slot = handle::slot;

  auto& description(std::string Desc) { return handle::description(Desc), *this; }
  auto& alias(std::string Alias) { return handle::alias(Alias), *this; }

  bool value() const {
    const auto sp = static_cast<slot*>(handle::_slots.get(id));
    if (sp && _parsed) return sp->has_value;
    throw unexpected_error(errors::argument_invalid_usage, "No value available");
  }
};

template<typename T> inline handle::positional<T> positional(std::string Metavar) {
  const auto id = handle::_slots
}
} // namespace yw::argument
