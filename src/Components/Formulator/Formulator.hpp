#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <variant>
#include <vector>

template <typename> struct TypeIdHack {};

inline void default_destructor(void *) {};

class Any {
  union {
    bool Bool;
    int Int;
    float Float;
    void *Ptr;
  } data;
  size_t type_hash = 0; // from typeid().hash_code()
  std::function<void(void *)> destructor = default_destructor;

public:
  inline Any() { data.Ptr = nullptr; }
  inline ~Any() {
    if (data.Ptr)
      destructor(data.Ptr);
  }

  Any(const Any &other) = delete;
  Any &operator=(const Any &other) = delete;

  inline Any(Any &&other) {
    data = other.data;
    type_hash = other.type_hash;
    destructor = other.destructor;
    other.data.Ptr = nullptr;
    other.type_hash = 0;
    other.destructor(other.data.Ptr);
    other.destructor = default_destructor;
  }

  inline Any &operator=(Any &&other) {
    if (this != &other) {
      data = other.data;
      type_hash = other.type_hash;
      destructor = other.destructor;
      other.data.Ptr = nullptr;
      other.type_hash = 0;
      other.destructor(other.data.Ptr);
      other.destructor = default_destructor;
    }
    return *this;
  }

  template <typename T> bool is_type() {
    return type_hash == typeid(TypeIdHack<T>).hash_code();
  }

  template <typename T> bool is_same_type(const T &other) {
    return type_hash == typeid(TypeIdHack<T>).hash_code();
  }

private:
  template <typename T> void put_base(const T &value) {
    *((T *)data.Ptr) = value;
    type_hash = typeid(TypeIdHack<T>);
    destructor = [](void *ptr) { delete (T *)ptr; };
  }

  template <> inline void put_base<bool>(bool value) {
    data.Bool = value;
    destructor = default_destructor;
  }

  template <> inline void put_base<int>(int value) {
    data.Int = value;
    destructor = default_destructor;
  }

  template <> inline void put_base<float>(float value) {
    data.Float = value;
    destructor = default_destructor;
  }

  template <typename T> void put_base(T &&value) {
    *((T *)data.Ptr) = value;
    type_hash = typeid(TypeIdHack<T>);
    destructor = [](void *ptr) { delete (T *)ptr; };
  }

  template <> inline void put_base<bool>(bool &&value) {
    data.Bool = value;
    destructor = default_destructor;
  }

  template <> inline void put_base<int>(int &&value) {
    data.Int = value;
    destructor = default_destructor;
  }

  template <> inline void put_base<float>(float &&value) {
    data.Float = value;
    destructor = default_destructor;
  }

  template <typename T> T *get_base() { return (T *)data.Ptr; }
  template <> inline bool *get_base() { return &data.Bool; };
  template <> inline int *get_base() { return &data.Int; }
  template <> inline float *get_base() { return &data.Float; }

public:
  template <typename T> void put(const T &value) {
    if (!is_type<T>()) {
      destructor(data.Ptr);
      data.Ptr = new T();
    }
    put_base<T>(value);
    type_hash = typeid(TypeIdHack<bool>);
  }

  template <typename T> void put(T &&value) {
    if (!is_type<T>()) {
      destructor(data.Ptr);
      data.Ptr = new T();
    }
    put_base<T>(value);
    type_hash = typeid(TypeIdHack<bool>);
  }

  template <typename T> T *get() {
    if (!is_type<T>())
      return nullptr;
    return get_base<T>();
  }
};
