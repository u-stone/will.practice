#pragma once
#include <iostream>
#include <string>
#include <vector>

struct A {
  std::string s;

  A(std::string str) : s(std::move(str)) { std::cout << " constructed\n"; }

  A(const A& o) : s(o.s) { std::cout << " copy constructed\n"; }

  A(A&& o) : s(std::move(o.s)) { std::cout << " move constructed\n"; }

  A& operator=(const A& other) {
    s = other.s;
    std::cout << " copy assigned\n";
    return *this;
  }

  A& operator=(A&& other) {
    s = std::move(other.s);
    std::cout << " move assigned\n";
    return *this;
  }
};

void vector_entry() {
  std::cout << "\n==========\n" << __FUNCTION__ << std::endl;

  std::vector<A> container;
  // reserve enough place so vector does not have to resize
  container.reserve(10);
  std::cout << "construct 2 times A:\n";

  A two{"two"};
  A three{"three"};

  std::cout << "emplace:\n";
  container.emplace(container.end(), "one");
  /*
  emplace:
  constructed
  */

  std::cout << "emplace with A&:\n";
  container.emplace(container.end(), two);
  /*
  emplace with A&:
  copy constructed
  */

  std::cout << "emplace with A&&:\n";
  container.emplace(container.end(), std::move(three));
  /*
  emplace with A&&:
  move constructed
  */

  std::cout << "push_back:\n";
  // container.push_back("another one");
  /*
  error C2664: 'void std::vector<A,std::allocator<_Ty>>::push_back(_Ty &&)':
  cannot convert argument 1 from 'const char [12]' to 'const _Ty &'
  */
  container.push_back({"another one"});
  /*
  push_back:
  constructed
  move constructed
  */

  std::cout << "push_back A&:\n";
  container.push_back(two);
  /*
  push_back A&:
  copy constructed
  */

  std::cout << "push_back A&&:\n";
  container.push_back(std::move(three));
  /*
  push_back A&&:
  move constructed
  */

  container.emplace_back("four");
}