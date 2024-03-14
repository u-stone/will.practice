#pragma once
#include <type_traits>

template <typename T>
class DxPtr {
  // static_assert(!std::is_pointer(T));
  typedef T DxType;
  typedef DxType* DxPointer;
  typedef DxPointer* DxPointerRef;

 public:
  DxPtr() : obj_(nullptr) {
    if (!count_) {
      count_ = new int(1);
    } else {
      ++*count_;
    }
  }

  DxPtr(T* t) : obj_(t) {
    if (!count_) {
      count_ = new int(1);
    } else {
      ++*count_;
    }
  }

  DxPtr(nullptr_t) : obj_(nullptr) {
    if (!count_) {
      count_ = new int(1);
    } else {
      ++*count_;
    }
  }

  DxPtr(const DxPtr& rh) {
    obj_ = rh.obj_;
    count_ = rh.count_;
    ++*count_;
  }

  DxPtr(DxPtr&& rh) noexcept : obj_(rh.obj_), count_(rh.count_) {
    rh.obj_ = nullptr;
    rh.count_ = nullptr;
  }

  DxPtr& operator=(const DxPtr& rh) {
    DxPtr(rh).swap(*this);
    return *this;
  }

  DxPtr& operator=(DxPtr&& rh) {
    DxPtr(std::move(rh)).swap(this);
    return *this;
  }

  ~DxPtr() {
    if (count_ && --*count_ == 0) {
      if (obj_) {
        obj_->Release();
        obj_ = nullptr;
      }

      delete count_;
      count_ = nullptr;
    }
  }

  operator bool() { return !!obj_; }

  operator T*() { return obj_; }

  T* get() { return obj_; }

  T** operator&() { return &obj_; }

  T* operator->() { return obj_; }

  void swap(DxPtr& rh) {
    std::swap(obj_, rh.obj_);
    std::swap(count_, rh.count_);
  }

 private:
  DxPointer obj_;
  int* count_ = nullptr;
};