#pragma once
#include <forward_list>

// std::forward_list is a container that supports fast insertion and removal of
// elements from anywhere in the container. Fast random access is not supported.
// It is implemented as a singly-linked list. Compared to std::list this
// container provides more space efficient storage when bidirectional iteration
// is not needed.

template <typename T>
std::ostream& operator<<(std::ostream& ostr, std::forward_list<T> flist) {
  for (auto& e : flist) {
    ostr << e << " ";
  }
  return ostr;
}

void forward_list_entry() {
  std::cout << "\n==========\n" << __FUNCTION__ << std::endl;

  std::forward_list<int> ints{1, 2, 3, 4, 5};
  std::cout << "ints:" << ints << std::endl;
  std::forward_list<char> chars{'a', 'b', 'c', 'd', 'e'};
  std::cout << "chars:" << chars << std::endl;

  auto iter = chars.begin();
  chars.insert_after(iter, 'f');
  std::cout << "chars:" << chars << std::endl;
}