#pragma once
#include <cstdint>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <variant>
#include <vector>

template <typename> struct TypeIdHack {};

class AnyClassStorage;

class AnyH {
  union {
    bool Bool;
    int Int;
    float Float;
    int32_t slice_idx; // maybe not slice but idx to T in std::vector<T>
  } data;
  size_t type_hash; // from typeid().hash_code()

  inline void set(bool value) { data.Bool = value; }
  inline void set(int value) { data.Int = value; }
  inline void set(float value) { data.Float = value; }

public:
  template <typename T> bool is_type() {
    return type_hash == typeid(TypeIdHack<T>).hash_code();
  }

  template <typename T> bool is_same_type(const T &other) {
    return type_hash == typeid(TypeIdHack<T>).hash_code();
  }

  template <typename T> put(T value, AnyClassStorage *storage);
};

class ArrayBase {
public:
  virtual ~ArrayBase() = 0;
};

template <typename T> class TypedArray : public ArrayBase {
public:
  std::vector<T> vector;

  TypedArray() = default;
  ~TypedArray() override = default;
};

class AnyClassStorage {
  std::unordered_map<size_t, std::unique_ptr<ArrayBase>> pools;

public:
  AnyClassStorage() = default;
  ~AnyClassStorage() = default;

  template <typename T> int32_t add() {}
  template <typename T> T *get(int32_t idx);
  template <typename T> void remove(int32_t idx);
};

template <typename T> AnyH::put(T value) { set(value); }
