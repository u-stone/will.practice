// cpp_test.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//

#include <iostream>

class Something {
 private:
  int x;

 public:
  Something(int xx) noexcept : x(xx) {}
  int getX() noexcept;
  void reset() noexcept { x = 0; }
};
int Something::getX() noexcept {
  if (x == 3)
    throw std::exception("I refuse to return 3");
  return x;
}
int main() {
  Something t(2);
  t.getX();
  return 0;
}
