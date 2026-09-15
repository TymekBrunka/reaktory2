#pragma once
#include <Eval.hpp>

class DataBinding {
  bool dirty;
  Eval::Value cached = None{};
  Eval::ASTnode ast;
};
