#define ywlib_disable_windows
#define ywlib_disable_direct3d
#include "ywlib"
using namespace yw;


// inline constexpr auto add_all = [](auto... args) { return (args + ...); };

// int main() {
//   std::cout << add_all(1, 2, 3, 4, 5) << std::endl;
//   std::cout << invoke(add_all, 1, 2, 3, 4, 5) << std::endl;
//   auto temp = list<>::asref(1, 2, 3, 4, 5);
//   std::cout << get<1>(temp) << std::endl;
//   // std::cout << apply(add_all, 1, 2, 3, 4, 5) << std::endl;

//   /
// }

struct test {
  int a;
  int& b;
  int&& c;
};

int main() {
  int bb = 4;
  test t = {1, bb, 3};
  static_assert(!is_reference<decltype(t.a)>);
  static_assert(is_lvref<decltype(t.b)>);
  static_assert(is_rvref<decltype(t.c)>);
}
