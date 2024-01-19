// smart_pointer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "UniquePtr.h"
#include <string>

class Test {
public:
  Test() {
    std::cout << "ctor";
  }
  Test(Test&& right) {
    std::cout << "move copy ctor";
  }

  Test& operator=(const Test& right) {
    std::cout << "assginment ctor";
    return *this;
  }

  Test& operator=(Test&& right) {
    std::cout << "move assginment ctor";
    return *this;
  }

  ~Test() {
    std::cout << "dtor";
  }

};

Test make_test() {
  return Test();
}

int main()
{
  //UniquePtr<std::string> sp;
  //UniquePtr<std::string> sp0 = nullptr;
  //UniquePtr<std::string> sp1(new std::string("23234"));
  //UniquePtr<std::string> sp2 = std::move(sp);
    //std::cout << "Hello World!\n";
   //UniquePtr<std::string> sp3 = make_unique_<std::string>("123");
   //std::cout << *sp3;
  Test t = make_test();
  system("pause");
   return 0;
}
