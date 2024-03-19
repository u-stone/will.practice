#pragma once

template <typename T>
T max(T a, T b) {
  return b < a ? a : b;
}

template <typename RT, typename T1, typename T2>
RT max3(T1 a, T2 b) {
  return b < a ? a : b;
}

template <typename T1, typename T2>
auto max_auto(T1 a, T2 b) {
  return b < a ? a : b;
}

template <typename T1, typename T2>
std::common_type_t<T1, T2> max_common(T1 a, T2 b) {
  return b < a ? a : b;
}

#include <type_traits>

template <typename T1,
          typename T2,
          typename RT = std::decay_t<decltype(true ? T1() : T2())>>
RT max_default(T1 a, T2 b) {
  return b < a ? a : b;
}

template <typename T1,
          typename T2,
          typename RT = std::decay_t<decltype(true ? std::declval<T1>()
                                                   : std::declval<T2>())>>
RT max_default_util(T1 a, T2 b) {
  return b < a ? a : b;
}

template <typename T1, typename T2, typename RT = std::common_type_t<T1, T2>>
RT max_default_common(T1 a, T2 b) {
  return b < a ? a : b;
}

int max(int a, int b) {
  return b < a ? a : b;
}

void test_ft() {
  // explicitly set type.
  ::max<double>(4, 7.2);   // double
  ::max3<double>(4, 7.2);  // double, int, double
  // automatically deduce.
  ::max_auto(2, 3.14);  // double, int, double
  ::max_common(2, 3.14);

  ::max_default(2, 3.14);
  ::max_default_util(2, 3.14);
  ::max_default_common(2, 3.14);

  ::max(1, 2);
}