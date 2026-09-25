#pragma once
#include <memory>
#include <variant>

class FormulaValue {
  std::variant<int, float> data;
};

class Formula {
public:
  struct Impl;
  std::unique_ptr<Impl> impl;

  NodeGraph() = default;
  ~NodeGraph() = default;
};
