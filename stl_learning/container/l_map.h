#pragma once
#include <map>
#include <string>

void output_map(const std::map<int, std::string>& _map,
                const std::string& notes) {
  std::cout << "\n" << notes << std::endl;

  for (const auto& e : _map) {
    std::cout << e.first << ":" << e.second << std::endl;
  }
}

void map_entry() {
  std::cout << "\n==========\n" << __FUNCTION__ << std::endl;

  std::map<int, std::string> t;

  try {
    std::cout << t.at(1);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    /*
    output: invalid map<K, T> key
    */
  }

  std::cout << "t[1]:{" << t[1] << "}" << std::endl;
  // t[1]:{}

  std::cout << "max_size:" << t.max_size() << ", size:" << t.size()
            << std::endl;
  // max_size:230584300921369395, size:1

  output_map(t, "after access element 1");
  /*
  after access element 1
  1:
  */

  t.emplace(2, "hello");
  output_map(t, "after emplace 2");
  /*
  after emplace 2
  1:
  2:hello
  */

  t.emplace(std::make_pair(1, "world"));
  output_map(t, "after emplace make_pair <1, world>");
  /*
  after emplace make_pair <1, world>
  1:
  2:hello
  */

  t.insert(std::make_pair(1, "world"));
  output_map(t, "after insert make_pair<1, world>");
  /*
  after insert make_pair<1, world>
  1:
  2:hello
  */

  t.erase(1);
  output_map(t, "after erase 1");
  /*
  after erase 1
  2:hello
  */

  auto iter = t.lower_bound(1);
  std::cout << std::boolalpha
            << "1 element iter equals begin: " << (t.begin() == iter)
            << std::endl;
  // 1 element iter equals begin:true

  iter = t.upper_bound(2);
  std::cout << std::boolalpha << "iter == t.begin:" << (t.begin() == iter)
            << ", iter == t.end()" << (t.begin() == iter) << std::endl;
  // iter == t.begin:false, iter == t.end()false

  iter = t.find(2);
  std::cout << "find(2):" << (iter != t.end()) << std::endl;
  // find(2):true

  auto c = t.count(2);
  std::cout << "count(2) = " << c << std::endl;
}