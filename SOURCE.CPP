// for test

#include "YWLIB"

using namespace yw;

int main() {

  float a[4]{1, 2, 3, 4};
  auto b = _mm_loadu_ps(a);
  auto c = _mm_add_ps(b, _mm_set_ps1(3.f));
  _mm_storeu_ps(a, c);
  std::print("a = [{}, {}, {}, {}]\n", a[0], a[1], a[2], a[3]);

}
