#pragma once
#include <variant>

struct no_error {
  bool dummy = false;
};

template <typename T, typename E> struct Result {
  std::variant<E, T>
      variant; // ET instead of TE in of std::monostate error type

  Result() : variant({}) {};

  Result Ok(const T &ok) {
    variant.template emplace<1>(ok);
    return std::move(*this);
  }

  Result Error(const E &err) {
    variant.template emplace<0>(err);
    return std::move(*this);
  }

  Result Ok(T &&ok) {
    variant.template emplace<1>(ok);
    return std::move(*this);
  }

  Result Error(E &&err) {
    variant.template emplace<0>(err);
    return std::move(*this);
  }

  bool is_ok() { return variant.index() == 1; }

  bool is_err() { return variant.index() == 0; }

  const T &ok_or(const T &ok) {
    if (const T *ok_val = std::get_if<T>(&variant)) {
      return *const_cast<T *>(ok_val);
    }
    return ok;
  }

  T &&ok_or(T &&ok) {
    if (const T *ok_val = std::get_if<T>(&variant)) {
      return std::move(*ok_val);
    }
    return ok;
  }

  T &ok_raw() { return std::get<T>(variant); }

  E &err_raw() { return std::get<E>(variant); }
};
