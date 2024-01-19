#pragma once
#include <memory>
template <typename T>
class UniquePtr
{
public:
  UniquePtr(const UniquePtr<T>&) = delete;
  UniquePtr& operator=(const UniquePtr<T>&) = delete;

  UniquePtr(nullptr_t) {}

  UniquePtr& operator=(nullptr_t) {
    reset(nullptr);
    return *this;
  }

  UniquePtr() {}

  UniquePtr(T* ptr) {
    reset(ptr);
  }

  UniquePtr(UniquePtr&& obj) {
    reset(obj.release());
  }

  UniquePtr& operator=(UniquePtr&& obj) {
    if (this != &obj) {
      reset(obj.release());
    }

    return *this;
  }

  ~UniquePtr() {
    if (data_) {
      delete data_;
      data_ = nullptr;
    }
  }

  T operator*()const {
    return *data_;
  }

  T* operator->() const {
    return data_;
  }

  void reset(T* obj = nullptr) {
    T* tmp = data_;
    data_ = obj;
    if (tmp) {
      delete tmp;
    }
  }

  T* release() {
    T* tmp = data_;
    data_ = nullptr;
    return tmp;
  }

  void swap(UniquePtr& obj) {
    swap(data_, obj.data_);
  }

private:
  T* data_ = nullptr;
};


template <class T, class... Args>
UniquePtr<T> make_unique_(Args&& ...args) {
  return UniquePtr<T>(new T(std::forward<Args>(args)...));
}
