#include "ywstd.h"
using namespace yw;

font<12, L"Arial", 0, true, false> ff;

int main() {

  list<int4, fat4> a{1, 3.0f};
  list<int8, fat8> b;
  vassign(b, a);
  std::cout << b.first << "," << b.second << std::endl;
  return 0;
}
