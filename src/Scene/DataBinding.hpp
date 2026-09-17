#pragma once
#include <Eval.hpp>

template <typename T> struct DataBinding {
public:
  bool dirty = true;
  Eval::Value cached = Eval::Value{.data = Eval::None{}};
  Eval::ASTnode formula;

  DataBinding() : cached(Eval::Value{.data = T{}}) {};
  DataBinding(const T &t) : cached(Eval::Value{.data = t}) {};

  Eval::Value *operator->() {
    return &cached;
  }
};
