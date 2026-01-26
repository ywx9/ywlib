#pragma once
#include "yw/core.h"

/**
 * \note
 * On Windows, command-line arguments are obtained from the OS
 * (GetCommandLineW + CommandLineToArgvW) and converted to UTF-8.
 * The C runtime's argc/argv are not used as the authoritative source.
 */

namespace yw::args {

struct option {
  std::string name{};
  std::vector<std::string> values{};
};

inline std::string program_name{};
inline std::vector<option> options{};
inline std::vector<std::string> positionals{};

inline bool _is_option_token(std::string_view tok) { return tok != "-" && tok.starts_with("-"); }

inline std::optional<std::pair<std::string_view, std::string_view>> _split_eq(std::string_view tok) {
  if (auto i = tok.find('='); i == std::string_view::npos) return std::nullopt;
  else if (i == 0) return std::nullopt; // ignore "=xxx"
  else return std::pair<std::string_view, std::string_view>{tok.substr(0, i), tok.substr(i + 1)};
}

inline void _add_option(std::string_view name, std::string_view value) {
  if (auto it = std::ranges::find(options, name, &option::name); it != options.end()) it->values.emplace_back(value);
  else options.push_back(option{std::string(name), std::vector<std::string>{std::string(value)}});
}

template<stringable<char> S> inline void _parse(int argc, S* argv) {
  bool after_double_dash = false;
  for (int i = 1; i < argc; ++i) {
    std::string_view tok = argv[i];
    if (!after_double_dash && tok == "--") after_double_dash = true;
    else if (after_double_dash) positionals.emplace_back(tok);
    else if (!_is_option_token(tok)) positionals.emplace_back(tok);
    else if (auto kv = _split_eq(tok)) _add_option(kv->first, kv->second);
    else {
      std::string name(tok);
      if (i + 1 < argc) {
        std::string_view next = argv[i + 1];
        if (!_is_option_token(next)) _add_option(name, next), ++i;
        else if (next.size() >= 2 && next[0] == '-' && std::isdigit(next[1])) _add_option(name, next), ++i;
        else _add_option(name, "");
      } else _add_option(name, "");
    }
  }
}

inline std::expected<void, error_trace> _parse_win() {
#ifdef _WIN32
  int argc;
  auto argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
  if (!argv)
    return unexpected_error(errors::operation_failed, "args::parse: CommandLineToArgvW failed", int(::GetLastError()));
  program_name = unicode<char>(std::filesystem::path(argv[0]).stem().native());
  std::vector<std::string> args;
  args.reserve(argc);
  for (int i = 0; i < argc; ++i) args.emplace_back(unicode<char>(std::wstring_view(argv[i])));
  ::LocalFree(argv);
  _parse(argc, args.data());
#endif
  return {};
}

/// parses command line arguments
inline std::expected<void, error_trace> parse(int argc, char** argv) {
#if defined(_WIN32) || defined(_WIN64)
  return _parse_win();
#else
  program_name = std::filesystem::path(argv[0]).stem().native();
  _parse(argc, argv);
  return {};
#endif
}

inline bool has(stringable<char> auto&& key) {
  return std::ranges::find(options, std::string_view(key), &option::name) != options.end();
}

inline std::optional<std::string_view> value(stringable<char> auto&& key) {
  auto it = std::ranges::find(options, std::string_view(key), &option::name);
  if (it == options.end() || it->values.empty()) return std::nullopt;
  else return std::string_view(it->values.front());
}

inline std::vector<std::string_view> values(stringable<char> auto&& key) {
  std::vector<std::string_view> out;
  if (auto it = std::ranges::find(options, std::string_view(key), &option::name); it != options.end()) {
    out.reserve(it->values.size());
    for (auto& s : it->values) out.emplace_back(s);
    return out;
  } else return out;
}
} // namespace yw::args
