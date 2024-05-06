#include "ywlib"
using namespace yw;

int main() {
  list a{5, 12.0, 'C', true, nat(1)};
  std::cout << get<0>(a) << std::endl;
  std::cout << get<1>(a) << std::endl;
  std::cout << get<2>(a) << std::endl;
  std::cout << get<3>(a) << std::endl;
  std::cout << get<4>(a) << std::endl;
}
