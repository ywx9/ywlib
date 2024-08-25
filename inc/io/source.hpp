/// \file io/source.hpp

#pragma once

#ifndef YWLIB
#include <source_location>
#else
import std;
#endif


#include "../core.hpp"


export namespace yw {


/// struct to represent a source location
struct source {

  source(const char*) = delete;

  /// file name
  const cat1* const file;

  /// function name
  const cat1* const func;

  /// line number
  const int4 line;

  /// column number
  const int4 column;

  /// default constructor to initialize the information
  constexpr source(
    const cat1* file = __builtin_FILE(),
    const cat1* func = __builtin_FUNCTION(),
    const int4 line = __builtin_LINE(),
    const int4 column = __builtin_COLUMN())
    : file(file), func(func), line(line), column(column) {}
};


} ////////////////////////////////////////////////////////////////////////////// namespace yw


namespace std {

// formatter

template<typename Ct> struct formatter<yw::source, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::source& src, auto& ctx) const {
    if constexpr (std::is_same_v<Ct, char>)
      return formatter<basic_string<Ct>, Ct>::format(
        std::format("{}({},{})({})", src.file, src.line, src.column, src.func), ctx);
    else
      return formatter<basic_string<Ct>, Ct>::format(
        std::format(L"{}({},{})({})", src.file, src.line, src.column, src.func), ctx);
  }
};

} ////////////////////////////////////////////////////////////////////////////// namespace std
