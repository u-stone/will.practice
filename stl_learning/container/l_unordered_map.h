#pragma once
#include <iostream>
#include <unordered_map>

// std::unordered_map is an associative container that contains key-value pairs
// with unique keys. Search, insertion, and removal of elements have average
// constant-time complexity.
// Internally, the elements are not sorted in any
// particular order, but organized into buckets. Which bucket an element is
// placed into depends entirely on the hash of its key. Keys with the same hash
// code appear in the same bucket. This allows fast access to individual
// elements, since once the hash is computed, it refers to the exact bucket the
// element is placed into.
// Two keys are considered equivalent if the map's key
// equality predicate returns true when passed those keys. If two keys are
// equivalent, the hash function must return the same value for both keys.
template <typename T, typename U>
std::ostream& operator<<(std::ostream& ostr, std::unordered_map<T, U> umap) {
  // for (const auto& [key, value] : umap) {
  for (const std::pair<const T, const U>& n : umap) {
    std::cout << n.first << ":" << n.second << "\n";
  }
  return ostr;
}

void unordered_map_entry() {
  std::cout << "\n==========\n" << __FUNCTION__ << std::endl;
  std::unordered_map<std::string, std::string> u = {
      {"RED", "#FF0000"}, {"GREEN", "#00FF00"}, {"BLUE", "#0000FF"}};
  std::cout << u << std::endl;
  /*
  RED:#FF0000
  GREEN:#00FF00
  BLUE:#0000FF
  */

  u["BLACK"] = "#000000";
  std::cout << "after insert BLACK: \n" << u << std::endl;
  /*
  after insert BLACK:
  RED:#FF0000
  GREEN:#00FF00
  BLACK:#000000
  BLUE:#0000FF
  */

  // u.at("WHITE");
  // std::cout << "after insert WHITE: \n" << u << std::endl;

  std::cout << u.bucket("RED") << "," << u.bucket_count() << ","
            << u.max_bucket_count();
}