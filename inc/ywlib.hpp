#pragma once

#ifndef YWLIB
#include <algorithm>
#if _HAS_STATIC_RTTI
#include <any>
#endif // _HAS_STATIC_RTTI
#include <array>
#include <atomic>
#include <barrier>
#include <bit>
#include <bitset>
#include <charconv>
#include <chrono>
#include <codecvt>
#include <compare>
#include <complex>
#include <concepts>
#include <condition_variable>
#include <coroutine>
#include <deque>
#include <exception>
#include <execution>
#if _HAS_CXX23
#include <expected>
#endif // _HAS_CXX23
#include <filesystem>
#include <format>
#include <forward_list>
#include <fstream>
#include <functional>
#include <future>
#include <initializer_list>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <latch>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <new>
#include <numbers>
#include <numeric>
#include <optional>
#include <ostream>
#if _HAS_CXX23
#include <print>
#endif // _HAS_CXX23
#include <queue>
#include <random>
#include <ranges>
#include <ratio>
#include <regex>
#include <scoped_allocator>
#include <semaphore>
#include <set>
#include <shared_mutex>
#include <source_location>
#include <span>
#if _HAS_CXX23
#include <spanstream>
#endif // _HAS_CXX23
#include <sstream>
#include <stack>
#if _HAS_CXX23
#include <stacktrace>
#endif // _HAS_CXX23
#include <stdexcept>
#if _HAS_CXX23
#include <stdfloat>
#endif // _HAS_CXX23
#include <stop_token>
#include <streambuf>
#include <string>
#include <string_view>
#include <strstream>
#include <syncstream>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <variant>
#include <vector>
#include <version>

#include <cassert>
#include <cctype>
#include <cerrno>
#include <cfenv>
#include <cfloat>
#include <cinttypes>
#include <climits>
#include <clocale>
#include <cmath>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cuchar>
#include <cwchar>
#include <cwctype>
#else
import std;
#endif

// ywlib headers

#pragma warning(push)
#pragma warning(disable : 4459) // declaration of '...' hides global declaration

#include "core.hpp"
#include "io.hpp"

// #include "apply.hpp"
// #include "array.hpp"
// #include "chrono.hpp"
// #include "color.hpp"
// #include "comptr.hpp"
// #include "core.hpp"
// #include "directx.hpp"
// #include "dwrite.hpp"
// #include "exception.hpp"
// #include "file.hpp"
// #include "get.hpp"
// #include "input.hpp"
// #include "list.hpp"
// #include "logger.hpp"
// #include "main.hpp"

// #include "intrin.hpp"

// #include "projector.hpp"
// #include "sequence.hpp"
// #include "sha256.hpp"
// #include "source.hpp"
// #include "status.hpp"
// #include "string.hpp"
// #include "typepack.hpp"
// #include "utility.hpp"
// #include "value.hpp"
// #include "vassign.hpp"
// #include "vector.hpp"
// #include "windows.hpp"
// #include "xvector.hpp"

#pragma warning(pop)
