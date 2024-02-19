#include <iostream>

struct DivideBy {
  constexpr DivideBy(double d) : _value(d) {}
  constexpr auto operator()(auto a) { return a / _value; }
private:
  double _value;
};


struct DivideBy2 {
  constexpr DivideBy2(double d) : _value(d) {}
  constexpr auto operator()(auto a) requires requires {
    a + _value; a - _value; a * _value; a / _value; } {
    return a / _value;
  }
private:
  double _value;
};

int main() {


  std::cout << DivideBy(11)(33) << std::endl;
  std::cout << DivideBy2(11)(33) << std::endl;
}
