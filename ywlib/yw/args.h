#pragma once
#include "yw/core.h"

/**
 * \note
 * On Windows, command-line arguments are obtained from the OS
 * (GetCommandLineW + CommandLineToArgvW) and converted to UTF-8.
 * The C runtime's argc/argv are not used as the authoritative source.
 */

namespace yw {

inline class {
  static bool _is_option_token(std::string_view tok) { return tok != "-" && tok.starts_with("-"); }
  static std::optional<std::pair<std::string_view, std::string_view>> _split_eq(std::string_view tok) {
    if (auto i = tok.find('='); i == std::string_view::npos) return std::nullopt;
    else if (i == 0) return std::nullopt; // ignore "=xxx"
    else return std::pair<std::string_view, std::string_view>{tok.substr(0, i), tok.substr(i + 1)};
  }
  template<stringable<char> S> void _parse(int argc, S* argv) {
    bool after_double_dash = false;
    for (int i = 1; i < argc; ++i) {
      std::string_view tok = argv[i];
      if (!after_double_dash && tok == "--") after_double_dash = true;
      else if (after_double_dash) positionals.emplace_back(tok);
      else if (!_is_option_token(tok)) positionals.emplace_back(tok);
      else if (auto kv = _split_eq(tok)) options[std::string(kv->first)].push_back(std::string(kv->second));
      else {
        std::string key(tok);
        if (i + 1 < argc) {
          std::string_view next = argv[i + 1];
          if (!_is_option_token(next)) {
            options[key].push_back(std::string(next));
            ++i;
          } else if (next.size() >= 2 && next[0] == '-' && std::isdigit(next[1])) {
            options[key].push_back(std::string(next));
            ++i;
          } else options.emplace(std::move(key), std::vector<std::string>{});
        } else options.emplace(std::move(key), std::vector<std::string>{});
      }
    }
  }
  std::expected<void, error_trace> _parse_win(const source& sl) {
#ifdef _WIN32
    int argc;
    auto argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
    if (!argv)
      return unexpected_error(
        errors::operation_failed, "args::parse: CommandLineToArgvW failed", int(::GetLastError()));
    program_name = unicode<char>(std::filesystem::path(argv[0]).stem().native());
    std::vector<std::string> args;
    args.reserve(argc);
    for (int i = 0; i < argc; ++i) args.emplace_back(unicode<char>(std::wstring_view(argv[i])));
    ::LocalFree(argv);
    _parse(argc, args.data());
#endif
    return {};
  }

public:
  std::string program_name{};
  std::unordered_map<std::string, std::vector<std::string>> options{};
  std::vector<std::string> positionals{};

  /// parses command line arguments
  std::expected<void, error_trace> parse(int argc, char** argv) {
#if defined(_WIN32) || defined(_WIN64)
    return _parse_win();
#else
    program_name = std::filesystem::path(argv[0]).stem().native();
    _parse(argc, argv);
    return {};
#endif
  }

  bool has(stringable<char> auto&& key) const { return options.find(std::string_view(key)) != options.end(); }

  std::optional<std::string_view> value(stringable<char> auto&& key) const {
    if (auto it = options.find(std::string_view(key)); it == options.end() || it->second.empty()) return std::nullopt;
    else return std::string_view(it->second.front());
  }

  std::vector<std::string_view> values(stringable<char> auto&& key) const {
    std::vector<std::string_view> out;
    auto it = options.find(std::string_view(key));
    if (it == options.end()) return out;
    out.reserve(it->second.size());
    for (auto& s : it->second) out.emplace_back(s);
    return out;
  }
} args;
} // namespace yw
