#pragma once
#include <Eval.hpp>

template <typename T> struct DataBinding {
public:
  bool dirty;
  Eval::Value cached = Eval::None{};
  Eval::ASTnode formula;

  const T *try_get() { return std::get_if<T>(&cached.data); }
};
