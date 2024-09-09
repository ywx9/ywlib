// for test

#include "YWLIB"

using namespace yw;

int main() {
  using namespace intrin;
  float a[4]{1, 2, 3, 4};
  auto b = mm_loadu_ps(a);
  auto c = mm_add_ps(b, mm_set1_ps(3.f));
  mm_storeu_ps(a, c);
  std::print("a = [{}, {}, {}, {}]\n", a[0], a[1], a[2], a[3]);
}
