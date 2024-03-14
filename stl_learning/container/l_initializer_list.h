#pragma once
#include <initializer_list>
#include <iostream>
#include <vector>

template <class T>
struct S {
  std::vector<T> v;

  S(std::initializer_list<T> l) : v(l) {
    std::cout << "constructed with a " << l.size() << "-element list\n";
  }

  void append(std::initializer_list<T> l) {
    v.insert(v.end(), l.begin(), l.end());
  }

  std::pair<const T*, std::size_t> c_arr() const {
    return {&v[0], v.size()};  // copy list-initialization in return statement
                               // this is NOT a use of std::initializer_list
  }
};

void initializer_entry() {
  std::cout << "\n==========\n" << __FUNCTION__ << std::endl;

  // used to list-initialize an object
  S<int> s{1, 2, 3, 4, 5};

  // a brace-enclosed initializer list is used as the right operand of
  // assignment or as a function call argument
  s.append({6, 7, 8});

  // a brace-enclosed initializer list is bound to auto, including in a ranged
  // for loop.
  for (auto& e : {1, 2, 3}) {
    std::cout << e << ' ';
  }

  std::cout << std::endl;
}