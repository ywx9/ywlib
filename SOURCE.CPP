// for test

#include "YWLIB"

using namespace yw;

int main() {

  float a[4]{1, 2, 3, 4};
  auto b = mm::loadu_ps<mm::m128>(a);
  auto c = mm::add_ps(b, mm::set_ps1(3.f));
  mm::storeu_ps(a, c);
  std::print("a = [{}, {}, {}, {}]\n", a[0], a[1], a[2], a[3]);

}
