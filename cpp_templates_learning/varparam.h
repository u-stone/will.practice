#pragma once
#include <iostream>

template<typename T, typename... Types>
void print(T firstArg, Types... args) {
  //std::cout << sizeof...(Types) << std::endl;
  //std::cout << sizeof...(args) << std::endl;

  std::cout << firstArg << std::endl;
  print(args...);
}

template<typename C, typename... Idx>
void printElements(C const& coll, Idx... idx) {
  print(coll[idx]...);
}


void test_varparam() {
  print(1, 3.14, "hello");
}