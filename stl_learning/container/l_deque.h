#pragma once
#include <deque>
#include <iostream>
#include <string>


void deque_entry() {
  std::cout << "\n==========\n" << __FUNCTION__ << std::endl;

  std::deque<char> characters;
  auto print_deque = [&]() {
    for (char c : characters)
      std::cout << c << ' ';
    std::cout << '\n';
  };

  characters.assign(5, 'a');
  print_deque();
  // a a a a a

  characters.assign({'C', '+', '+', '1', '1'});
  print_deque();
  // C + + 1 1

  characters[0] = ' ';
  print_deque();
  //   + + 1 1

  /*
  try {
    characters[10] = '1';
  } catch (...) {
    std::cout << "catch out of range error" << std::endl;
  }
  */
  

  characters.at(4) = '2';

  std::cout << characters.max_size() << std::endl;

  characters.push_back('q');
  characters.push_front('w');
  print_deque();
  // w   + + 1 1 q
  characters.pop_back();
  characters.pop_front();
  print_deque();
  //    + + 1 1

  auto iter = characters.begin();
  characters.insert(iter, 'a');
  print_deque();
  // a   + + 1 2

  std::deque<char> another{'1', '2', '3'};
  characters.insert(characters.begin(), std::begin(another), std::end(another));
  print_deque();
  // 1 2 3 a   + + 1 2

  characters.insert(characters.end(), std::begin(another), std::end(another));
  print_deque();
  // 1 2 3 a   + + 1 2 1 2 3


}