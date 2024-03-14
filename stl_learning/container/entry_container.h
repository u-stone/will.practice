#pragma once

#include "l_array.h"
#include "l_deque.h"
#include "l_forward_list.h"
#include "l_initializer_list.h"
#include "l_list.h"
#include "l_map.h"
#include "l_multimap.h"
#include "l_multiset.h"
#include "l_set.h"
#include "l_unordered_map.h"
#include "l_unordered_multimap.h"
#include "l_unordered_multiset.h"
#include "l_unordered_set.h"
#include "l_vector.h"

// Some articles about STL: 
// https://blog.csdn.net/daaikuaichuan/article/details/80717222
// 
void run_container() {
  array_entry();

  map_entry();
  multimap_entry();
  unordered_map_entry();

  initializer_entry();

  list_entry();

  forward_list_entry();

  vector_entry();

  deque_entry();
}