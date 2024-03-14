#pragma once


#include <array>

void array_entry() {
  std::cout << "\n==========\n" << __FUNCTION__ << std::endl;

  std::array<int, 3> t {
    { 1, 2, 3 }};

  // a length-fixed array on stack.

  std::cout << t.at(1) << t[2] << t.front() << t.back() << std::endl;
  // 2313 

  std::cout << t.max_size() << t.size();
  // 33

}