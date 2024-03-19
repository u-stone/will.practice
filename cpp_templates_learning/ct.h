#pragma once
#include <string>
#include <vector>

template <typename T>
class Stack {
 private:
  std::vector<T> elements;

 public:
  void push(T const& e);
  void pop();
  T const& top();
  bool empty() const { return elements.empty(); }
};

template <typename T>
void Stack<T>::push(T const& e) {
  elements.emplace_back(e);
}

template <typename T>
void Stack<T>::pop() {
  elements.pop_back();
}

template <typename T>
T const& Stack<T>::top() {
  return elements.back();
}

// full specialization
template <>
class Stack<std::string> {
 private:
  std::vector<std::string> elements;

 public:
  void push(std::string const& e);
  void pop();
  std::string const& top();
  bool empty() const { return elements.empty(); }
};

void Stack<std::string>::push(std::string const& e) {
  elements.emplace_back(e);
}

void Stack<std::string>::pop() {
  elements.pop_back();
}

std::string const& Stack<std::string>::top() {
  return elements.back();
}

// partially specialization

// default parameter for class templetes.
template <class T, class Cont = std::vector<T>>
class Stack {
 private:
  Cont elements;

 public:
  void push(T& e) { elements.push_back(e); }
  void pop() { elements.pop(); }
  T const& top() { return elements.back(); }
  bool empty() const { return elements.empty(); }
};

// template <typename T, typename Cont = std::vector<T>>
// void Stack<T, Cont>::push(T const& e) {
//   elements.push_back(e);
// }
//
// template <typename T, typename Cont>
// void Stack<T, Cont>::pop() {
//   elements.pop();
// }
//
// template <typename T, typename Cont>
// T const& Stack<T, Cont>::top {
//   return elements.back();
// }

void test_ct() {
  Stack<int> s;
  s.push(1);
  s.pop();
  // s.top();
}
