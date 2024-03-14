#pragma once
#include <iostream>
#include <list>

std::ostream& operator<<(std::ostream& ostr, const std::list<int>& list) {
  for (auto& i : list)
    ostr << ' ' << i;

  return ostr;
}

void list_entry() {
  std::cout << "\n==========\n" << __FUNCTION__ << std::endl;


  std::list<int> l = {7, 5, 16, 8};

  l.push_back(1);
  l.push_front(0);

  auto print_list = [](auto& l) {
    for (auto& e : l) {
      std::cout << e << ' ';
    }
    std::cout << std::endl;
  };

  print_list(l);
  // 0 7 5 16 8 1

  std::cout << l.front() << ' ' << l.back();
  // 0 1

  std::list<int> list1 = {5, 9, 1, 3, 3};
  std::list<int> list2 = {8, 7, 2, 3, 4, 4};

  std::cout << "list1:\n";
  print_list(list1);
  list1.sort();
  std::cout << "list1 after sort:\n";
  print_list(list1);
  // 1 3 3 5 9

  list2.sort();
  std::cout << "list2 after sort:\n";
  print_list(list2);
  // 2 3 4 4 7 8

  list1.merge(list2);
  std::cout << "list1 after merge:\n";
  print_list(list1);
  // 1 2 3 3 3 4 4 5 7 8 9

  auto it = list1.begin();
  std::advance(it, 2);
  std::cout << "list1 splice:\n";
  list1.splice(it, list2);
  print_list(list1);
  // 1 2 3 3 3 4 4 5 7 8 9

  list1.unique();
  std::cout << "list1 uniqued: " << list1 << std::endl;
  // list1 uniqued:  1 2 3 4 5 7 8 9

  list1 = {1, 2, 3};
  list2 = {5, 6, 7};
  std::cout << "output list1:\n";
  std::cout << list1 << std::endl;
  //  1 2 3
  std::cout << "output list2:\n";
  std::cout << list2 << std::endl;
  //  5 6 7

  list1.reverse();
  std::cout << "list1 reversed:" << list1 << std::endl;
  // list1 reversed: 3 2 1
  list1.insert(list1.end(), 4);
  std::cout << "list1:" << list1 << std::endl;
  // list1: 3 2 1 4
}