#define ywlib_disable_windows
#define ywlib_disable_direct3d
#include "ywlib"
using namespace yw;

int main() {

  constexpr yw::make_sequence<10, 2, [](nat n) { return n * n; }> ms;
  std::cout << ms << std::endl;
}
