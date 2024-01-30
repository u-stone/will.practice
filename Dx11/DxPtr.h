#pragma once
#include <type_traits>

template<typename T>
class DxPtr {
  //static_assert(!std::is_pointer(T));
  typedef T DxType;
  typedef DxType* DxPointer;
  typedef DxPointer *DxPointerRef;
public:
  DxPtr() : obj(nullptr) {}

  DxPtr(T* t) :obj(t) {
  }

  DxPtr(nullptr_t) : obj(nullptr) {}

  ~DxPtr() {
    if (obj) {
      obj->Release();
      obj = nullptr;
    }
  }

  T* get() {
    return obj;
  }

  T** operator&() {
    return &obj;
  }

  T* operator->() {
    return obj;
  }

private:
  DxPointer obj;
};