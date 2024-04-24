#include "ywlib"
using namespace yw;

bool none_test = []() {
  std::cout << "starts none test" << std::endl;
  static_assert(std::common_with<none, int>);
  static_assert(std::common_reference_with<int, none>);
  std::cout << none{} << " <- none" << std::endl;
  std::cout << std::format("{} <- none", none{}) << std::endl;
  return std::cout << std::endl, true;
}();

bool value_test = []() {
  static_assert(std::same_as<std::common_type_t<value, int>, fat8>);
  std::cout << "Inputs a value: " << [](value v) { return std::cin >> v, v; }({}) << " <- input" << std::endl;
  return std::cout << std::endl, true;
}();

bool type_test = []() {

}();

int main() {
}
