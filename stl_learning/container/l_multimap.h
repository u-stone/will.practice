#pragma once
#include <map>

// print out a std::pair
template <class Os, class U, class V>
Os& operator<<(Os& os, const std::pair<U, V>& p) {
  return os << p.first << ':' << p.second;
}

// print out a container
template <class Os, class Co>
Os& operator<<(Os& os, const Co& co) {
  os << '{';
  for (auto const& i : co)
    os << ' ' << i;
  return os << " }\n";
}

void multimap_entry() {
  std::cout << "\n==========\n" << __FUNCTION__ << std::endl;

  std::multimap<std::string, std::string> m1{
      {"1", "gamma"}, {"2", "beta"}, {"3", "alpha"}, {"1", "gamma"}},
      m2{{"4", "epsilon"}, {"5", "delta"}, {"4", "epsilon"}};
  const auto& ref = *(m1.begin());
  const auto iter = std::next(m1.cbegin());

  std::cout << "before swap:\n"
            << "m1: " << m1 << "m2: " << m2 << "ref: " << ref
            << "\niter: " << *iter << '\n';
  /*
  before swap:
  m1: { 1:gamma 1:gamma 2:beta 3:alpha }
  m2: { 4:epsilon 4:epsilon 5:delta }
  ref: 1:gamma
  iter: 1:gamma
  */


}