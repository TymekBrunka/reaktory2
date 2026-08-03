#pragma once
namespace Errors {

typedef bool no_error;

template <typename T, typename E> struct Result {
  bool is_ok;
  union {
    T success;
    E error;
  } value;

  static Result OK(T success) {
    Result res;
    res.is_ok = true;
    res.value.success = success;
    return res;
  }

  static Result ERR(E error) {
    Result res;
    res.is_ok = false;
    res.value.error = error;
    return res;
  }

  template <typename TT, typename EE> Result(Result<TT, EE> other) {
    is_ok = other.is_ok;
    value = other.is_ok ? other.value.success : other.value.error;
  }

  T ok_unchecked() const { return value.success; }

  E err_unchecked() const { return value.error; }

  T ok_or(T fallback) const { return is_ok ? value.success : fallback; }

  T err_or(T fallback) const { return !is_ok ? value.error : fallback; }
};

} // namespace Errors
