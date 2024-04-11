#define ywlib_disable_windows
#define ywlib_disable_direct3d
#include "ywlib"
#include "ywlib-wrapper.h"
using namespace yw;

int main() {

  constexpr wrapper a(1);
  constexpr auto b = "test test";
  constexpr wrapper c(b);
}
