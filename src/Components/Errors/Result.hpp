#pragma once
#include <variant>

struct no_error {
  bool dummy = false;
};

template <typename T> struct Ok {
  T value;
  inline operator T &() { return value; }
};

template <typename T> struct Error {
  T value;
  inline operator T &() { return value; }
};

template <typename T, typename E> struct Result {
  using Variant = std::variant<Error<E>, Ok<T>>;
  Variant
      variant; // ET instead of TE in case of std::monostate/no_error error type

  Result() : variant({}) {};
  Result(Variant variant_) : variant(variant_) {};

  bool is_ok() { return variant.index() == 1; }

  bool is_err() { return variant.index() == 0; }

  const T &ok_or(const T &ok) {
    if (const T *ok_val = std::get_if<Ok<T>>(&variant)) {
      return *const_cast<T *>(ok_val);
    }
    return ok;
  }

  T &&ok_or(T &&ok) {
    if (const T *ok_val = std::get_if<Error<T>>(&variant)) {
      return std::move(*ok_val);
    }
    return ok;
  }

  T &ok_raw() { return std::get<Ok<T>>(variant); }

  E &err_raw() { return std::get<Error<E>>(variant); }
};
