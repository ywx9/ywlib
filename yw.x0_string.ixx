module;
#include "yw.hpp"
export module yw.x0 :string;
import <string>;
export namespace yw {
  export template<character T> using view_string = std::basic_string_view<T>;
  export template<character T> using string = std::basic_string<T>;
  export template<character T> using char_traits = std::char_traits<T>;
  export using str1 = string<cat1>;
  export using str2 = string<cat2>;
  export using strt = string<catt>;

  export template<is_cat C, quantable T, natt Base = 10> [[nodiscard]] T stov(const string<C>& str) {
    if constexpr (is_int<T>) {
      if constexpr (same_as<T, int4>) return std::stoi(str, nullptr, Base);
      else if constexpr (same_as<T, int8>) return std::stoll(str, nullptr, Base);
      else return static_cast<T>(std::stoi(str, nullptr, Base));
    }
    else if constexpr (is_nat<T>) {
      if constexpr (same_as<T, nat8>) return std::stoull(str, nullptr, Base);
      else return static_cast<T>(std::stoul(str, nullptr, Base));
    }
    else if constexpr (is_fat<T>) {
      if constexpr (same_as<T, fat4>) return std::stof(str, nullptr);
      else if constexpr (same_as<T, fat8>) return std::stod(str, nullptr);
      else return static_cast<T>(std::stod(str, nullptr));
    }
    else { static_assert(false); return T(); }
  }

  export template<is_cat C, quantable T> [[nodiscard]] string<C> vtos(T val) {
    if constexpr (same_as<C, cat1>) return std::to_string(val);
    else if constexpr (same_as<C, cat2>) return std::to_string(val);
    else { static_assert(false); return string{}; }
  }
}
