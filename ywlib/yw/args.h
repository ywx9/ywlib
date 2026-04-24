#pragma once
#include "yw/error.h"

#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <memory>
#include <optional>
#include <unordered_map>

#ifdef _WIN32
#include <shellapi.h>
#endif

/**
 * \note
 * On Windows, command-line arguments are obtained from the OS
 * (GetCommandLineW + CommandLineToArgvW) and converted to UTF-8.
 * The C runtime's argc/argv are not used as the authoritative source.
 */

namespace yw {
define_error(argument_unknown_option);
define_error(argument_missing_value);
define_error(argument_invalid_value);
define_error(argument_missing_required);
define_error(argument_duplicated_option);
define_error(argument_too_many);
define_error(argument_too_few);
} // namespace yw

namespace yw {

namespace detail {

inline bool is_option_token(std::string_view tok) { return tok != "-" && tok.starts_with('-'); }

inline bool is_negative_number_token(std::string_view tok) {
  if (tok.size() < 2 || tok[0] != '-') return false;
  return std::isdigit(static_cast<unsigned char>(tok[1])) || tok[1] == '.';
}

inline std::optional<std::pair<std::string_view, std::string_view>> split_eq(std::string_view tok) {
  if (auto i = tok.find('='); i == std::string_view::npos) return std::nullopt;
  else if (i == 0) return std::nullopt;
  else return std::pair<std::string_view, std::string_view>{tok.substr(0, i), tok.substr(i + 1)};
}

inline std::string normalize_name(std::string_view name) {
  if (name.empty()) return {};
  if (name[0] == '-') return std::string(name);
  if (name.size() == 1) return format("-{}", name);
  return format("--{}", name);
}

inline std::vector<std::string> split_names(std::string_view names) {
  std::vector<std::string> out;
  size_t i = 0;
  while (i < names.size()) {
    while (
      i < names.size() && (names[i] == ',' || names[i] == '|' || std::isspace(static_cast<unsigned char>(names[i])))) {
      ++i;
    }
    const size_t beg = i;
    while (
      i < names.size() && names[i] != ',' && names[i] != '|' && !std::isspace(static_cast<unsigned char>(names[i]))) {
      ++i;
    }
    if (beg < i) out.push_back(normalize_name(names.substr(beg, i - beg)));
  }
  if (out.empty()) out.push_back(normalize_name(names));
  return out;
}

class item_base {
public:
  virtual ~item_base() = default;
  virtual void reset() const = 0;
  virtual std::expected<void, error_trace> finalize() const = 0;
};

class named_base : public item_base {
public:
  std::vector<std::string> names{};
  bool required{false};
  bool multiple{false};
  std::string description{};
  std::string metavar{};

  virtual bool accepts_value() const = 0;
  virtual std::expected<void, error_trace> add(std::string_view raw_name, std::string_view value) const = 0;
};

class positional_base : public item_base {
public:
  std::string name{};
  bool required{true};
  bool multiple{false};
  std::string description{};
  std::string metavar{};

  virtual bool empty() const = 0;
  virtual std::expected<void, error_trace> add(std::string_view value) const = 0;
};

inline std::expected<std::vector<std::string>, error_trace> collect_argv(
  int argc, char** argv, std::string& program_name) {
  std::vector<std::string> args;

#if defined(_WIN32) || defined(_WIN64)
  int argcw = 0;
  auto argvw = ::CommandLineToArgvW(::GetCommandLineW(), &argcw);
  if (!argvw)
    return unexpected_error(errors::operation_failed, "argument: CommandLineToArgvW failed", int(::GetLastError()));

  args.reserve(static_cast<size_t>(argcw));
  for (int i = 0; i < argcw; ++i) args.emplace_back(unicode<char>(std::wstring_view(argvw[i])));
  ::LocalFree(argvw);
#else
  args.reserve(static_cast<size_t>(argc));
  for (int i = 0; i < argc; ++i) args.emplace_back(argv[i]);
#endif

  if (!args.empty()) program_name = std::filesystem::path(args.front()).stem().string();
  return args;
}

} // namespace detail

template<typename T> struct converter {
  static std::expected<T, error_trace> convert(std::string_view) {
    return unexpected_error(errors::argument_invalid_value, "Invalid value for argument");
  }
};

template<> struct converter<std::string> {
  static std::expected<std::string, error_trace> convert(std::string_view value) { return std::string(value); }
};

template<> struct converter<bool> {
  static std::expected<bool, error_trace> convert(std::string_view value) {
    if (value == "1" || value == "true" || value == "on" || value == "yes") return true;
    if (value == "0" || value == "false" || value == "off" || value == "no") return false;
    return unexpected_error(errors::argument_invalid_value, format("Invalid bool: {}", value));
  }
};

template<arithmetic T> struct converter<T> {
  static std::expected<T, error_trace> convert(std::string_view value) {
    T out{};
    const auto* b = value.data();
    const auto* e = b + value.size();

    if constexpr (integral<T> && !is_bool<T>) {
      if (auto res = std::from_chars(b, e, out); res.ec != std::errc() || res.ptr != e)
        return unexpected_error(errors::argument_invalid_value, format("Invalid integer: {}", value));
      return out;
    } else if constexpr (floating<T>) {
      std::string s(value);
      char* end = nullptr;
      errno = 0;
      const auto v = std::strtold(s.c_str(), &end);
      if (errno != 0 || end != s.c_str() + s.size())
        return unexpected_error(errors::argument_invalid_value, format("Invalid number: {}", value));
      return static_cast<T>(v);
    } else {
      static_assert(always_false<T>, "unsupported arithmetic type");
    }
  }
};
} // namespace yw

namespace yw {
inline class {
public:
  template<typename T> class option_t : public detail::named_base {
  public:
    std::optional<T> _default{};
    mutable std::vector<T> _values{};
    mutable bool _specified{false};

    explicit option_t(std::string name) { names = detail::split_names(name); }
    bool accepts_value() const override { return true; }

    std::expected<void, error_trace> add(std::string_view raw_name, std::string_view value) const override {
      if (!detail::named_base::multiple && !_values.empty())
        return unexpected_error(errors::argument_duplicated_option, format("Duplicated option: {}", raw_name));

      auto cv = converter<T>::convert(value);
      if (!cv)
        return unexpected_error(
          errors::argument_invalid_value, format("Invalid value for {}: {}", raw_name, value));

      _values.push_back(std::move(*cv));
      _specified = true;
      return {};
    }

    void reset() const override {
      _values.clear();
      _specified = false;
    }

    std::expected<void, error_trace> finalize() const override {
      if (detail::named_base::required && !_specified)
        return unexpected_error(
          errors::argument_missing_required, format("Missing required option: {}", names.front()));
      if (_values.empty() && _default) _values.push_back(*_default);
      return {};
    }

    option_t& short_name(char c) { return names.push_back(format("-{}", c)), *this; }
    option_t& required(bool on = true) { return detail::named_base::required = on, *this; }
    option_t& default_value(T v) { return _default = std::move(v), *this; }
    option_t& description(std::string s) { return detail::named_base::description = std::move(s), *this; }
    option_t& metavar(std::string s) { return detail::named_base::metavar = std::move(s), *this; }
    option_t& multiple(bool on = true) { return detail::named_base::multiple = on, *this; }

    [[nodiscard]] const std::vector<T>& values() const { return _values; }
    [[nodiscard]] const T& value() const {
      if (!_values.empty()) return _values.front();
      if (_default) return *_default;
      static const T empty{};
      return empty;
    }
    [[nodiscard]] bool has_value() const { return _specified; }
  };

  class flag_t : public detail::named_base {
  public:
    mutable bool _set{false};

    explicit flag_t(std::string name) { names = detail::split_names(name); }

    bool accepts_value() const override { return false; }

    std::expected<void, error_trace> add(std::string_view raw_name, std::string_view value) const override {
      if (!value.empty())
        return unexpected_error(
          errors::argument_invalid_value, format("Flag does not accept value: {}", raw_name));
      if (_set)
        return unexpected_error(errors::argument_duplicated_option, format("Duplicated option: {}", raw_name));

      _set = true;
      return {};
    }

    void reset() const override { _set = false; }

    std::expected<void, error_trace> finalize() const override {
      if (detail::named_base::required && !_set)
        return unexpected_error(
          errors::argument_missing_required, format("Missing required flag: {}", names.front()));
      return {};
    }

    flag_t& short_name(char c) { return names.push_back(format("-{}", c)), *this; }
    flag_t& required(bool on = true) { return detail::named_base::required = on, *this; }
    flag_t& description(std::string s) { return detail::named_base::description = std::move(s), *this; }

    [[nodiscard]] bool value() const { return _set; }
    [[nodiscard]] explicit operator bool() const { return _set; }
  };

  template<typename T> class positional_t : public detail::positional_base {
  public:

    mutable std::vector<T> _values{};

    explicit positional_t(std::string name) { this->name = std::move(name), this->metavar = this->name; }

    bool empty() const override { return _values.empty(); }

    std::expected<void, error_trace> add(std::string_view value) const override {
      if (auto cv = converter<T>::convert(value)) _values.push_back(std::move(*cv));
      else return unexpected_error(errors::argument_invalid_value, format("Invalid value for {}: {}", name, value));
      return {};
    }

    void reset() const override { _values.clear(); }

    std::expected<void, error_trace> finalize() const override {
      if (detail::positional_base::required && _values.empty())
        return unexpected_error(errors::argument_too_few, format("Missing positional argument: {}", name));
      return {};
    }

    positional_t& required(bool on = true) { return detail::positional_base::required = on, *this; }
    positional_t& multiple(bool on = true) { return detail::positional_base::multiple = on, *this; }
    positional_t& description(std::string s) { return detail::positional_base::description = std::move(s), *this; }
    positional_t& metavar(std::string s) { return detail::positional_base::metavar = std::move(s), *this; }

    [[nodiscard]] const std::vector<T>& values() const { return _values; }
    [[nodiscard]] std::optional<T> value() const {
      if (_values.empty()) return std::nullopt;
      return _values.front();
    }
  };

private:
  mutable std::string _program_name{};
  std::vector<std::unique_ptr<detail::item_base>> _items{};
  std::vector<detail::named_base*> _named{};
  std::vector<detail::positional_base*> _positionals{};

public:
  template<typename T> option_t<T>& option(std::string name) {
    auto item = std::make_unique<option_t<T>>(std::move(name));
    auto* raw = item.get();
    _named.push_back(raw);
    _items.push_back(std::move(item));
    return *raw;
  }

  flag_t& flag(std::string name) {
    auto item = std::make_unique<flag_t>(std::move(name));
    auto* raw = item.get();
    _named.push_back(raw);
    _items.push_back(std::move(item));
    return *raw;
  }

  template<typename T> positional_t<T>& positional(std::string name) {
    auto item = std::make_unique<positional_t<T>>(std::move(name));
    auto* raw = item.get();
    _positionals.push_back(raw);
    _items.push_back(std::move(item));
    return *raw;
  }

  [[nodiscard]] std::string_view program_name() const noexcept { return _program_name; }

  std::expected<void, error_trace> parse(int argc, char** argv) const {
    for (const auto& item : _items) item->reset();

    auto args = detail::collect_argv(argc, argv, _program_name);
    if (!args) return std::unexpected<error_trace>(args.error());

    std::unordered_map<std::string_view, detail::named_base*> name_map;
    for (auto* n : _named) {
      for (const auto& name : n->names) {
        if (name.empty()) continue;
        if (auto [it, ok] = name_map.emplace(name, n); !ok)
          return unexpected_error(
            errors::invalid_argument, format("Duplicated option name registration: {}", name));
      }
    }

    bool after_double_dash = false;
    size_t positional_index = 0;

    for (size_t i = 1; i < args->size(); ++i) {
      const std::string_view tok = (*args)[i];

      if (!after_double_dash && tok == "--") {
        after_double_dash = true;
        continue;
      }

      if (!after_double_dash && detail::is_option_token(tok)) {
        std::string_view name = tok;
        std::string_view value{};
        bool has_value = false;

        if (auto kv = detail::split_eq(tok)) {
          name = kv->first;
          value = kv->second;
          has_value = true;
        }

        auto it = name_map.find(name);
        if (it == name_map.end())
          return unexpected_error(errors::argument_unknown_option, format("Unknown option: {}", tok));

        auto* spec = it->second;
        if (spec->accepts_value()) {
          if (!has_value && i + 1 < args->size()) {
            const std::string_view next = (*args)[i + 1];
            if (!detail::is_option_token(next) || detail::is_negative_number_token(next)) {
              value = next;
              has_value = true;
              ++i;
            }
          }
          if (!has_value)
            return unexpected_error(errors::argument_missing_value, format("Missing value for option: {}", name));
        } else if (has_value)
          return unexpected_error(errors::argument_invalid_value, format("Flag does not accept value: {}", name));

        if (auto r = spec->add(name, value); !r) return std::unexpected<error_trace>(r.error());
        continue;
      }

      while (positional_index < _positionals.size() && !_positionals[positional_index]->multiple &&
             !_positionals[positional_index]->empty()) {
        ++positional_index;
      }

      if (positional_index >= _positionals.size())
        return unexpected_error(errors::argument_too_many, format("Unexpected positional argument: {}", tok));

      if (auto r = _positionals[positional_index]->add(tok); !r) return std::unexpected<error_trace>(r.error());
    }

    for (const auto& item : _items)
      if (auto r = item->finalize(); !r) return std::unexpected<error_trace>(r.error());

    return {};
  }
} argument;
} // namespace yw
