#pragma once
#include <Model.hpp>
#include <Renderer.hpp>
#include <ResourceManager.hpp>
#include <functional>
#include <string>
#include <unordered_map>
#include <variant>
namespace Eval {

struct Value;

class Class {
public:
  virtual const char *Name() = 0;
  virtual const std::vector<const char *> &Members() = 0;
  virtual const std::vector<const char *> &Methods() = 0;
  virtual Value Get(const std::string &name) = 0;
  virtual Value Call(const std::string &name,
                     const std::vector<Value> &args) = 0;
};

class StringWrap : Class {
public:
  std::string data;

  inline const char *Name() override { return "Ciąg znaków"; }
  inline const std::vector<const char *> &Members() override {
    static std::vector<const char *> vec{"długość", "znaki"};
    return vec;
  }
  inline const std::vector<const char *> &Methods() override {
    static std::vector<const char *> vec{"długość", "podciąg", "szukaj",
                                         "szukaj_od_prawej", "zawiera"};
    return vec;
  }

  inline Value Get(const std::string &name) override;
  Value Call(const std::string &name, const std::vector<Value> &args) override;
};

struct None {
  bool dummy = false;
};

struct Model {
  std::string name;
};

struct Material {
  glm::vec4 color{};
  std::string diffuse1;
};

struct objHFormula {
  uint8_t gen;
  // uint16_t idx;
  int16_t idx; // signed integer so -1 can be returned
  bool is_formula = false;
};

struct Value {
  std::string err_msg;
  std::variant<None, std::shared_ptr<StringWrap>, std::shared_ptr<Model>,
               std::shared_ptr<Material>, objHFormula, std::shared_ptr<Class>,
               int, float, bool, std::shared_ptr<std::vector<Value>>>
      data;

  template <typename T> inline T *get_shared() {
    auto *ptr = (std::shared_ptr<T> *)std::get_if<std::shared_ptr<T>>(&data);

    return ptr ? ptr->get() : nullptr;
  }

  inline std::vector<Value> *get_vector() {
    return get_shared<std::vector<Value>>();
  }

  inline std::string *get_string() {
    auto *ptr = get_shared<StringWrap>();
    return ptr ? &ptr->data : nullptr;
  }

  inline std::string *get_model() {
    auto *ptr = get_shared<Model>();
    return ptr ? &ptr->name : nullptr;
  }

  inline Material *get_material() { return get_shared<Material>(); }

  template <typename T> inline T *get() { return (T *)std::get_if<T>(&data); }
};

inline Value StringWrap::Get(const std::string &name) {
  if (name == "długość") {
    return Value{.data = (int)data.size()};
  }

  return Value{.data = None{}};
}

#define EVAL_ast_tag_names                                                     \
  X(ROOT)                                                                      \
  X(CALL)                                                                      \
  X(GET)                                                                       \
  X(SET)                                                                       \
  X(ADD)                                                                       \
  X(SUB)                                                                       \
  X(MUL)                                                                       \
  X(DIV)                                                                       \
  X(FIELD_ACCESS)                                                              \
  X(NESTED_EXPR)                                                               \
  X(EQUAL)                                                                     \
  X(NOT)                                                                       \
  X(NOT_EQUAL)                                                                 \
  X(LESS_THAN)                                                                 \
  X(MORE_THAN)                                                                 \
  X(LESS_OR_EQUALS)                                                            \
  X(MORE_OR_EQUALS)                                                            \
  X(INVALID)

#define X(x) a##x,
enum ASTtag { EVAL_ast_tag_names };
#undef X

struct ASTnode {
  ASTtag tag = aROOT;
  uint32_t node_data_idx;
  std::vector<ASTnode> children;
};

#define EVAL_bc_tag_names                                                      \
  X(SLOAD)                                                                     \
  X(SSTORE)                                                                    \
  X(SMOV)                                                                      \
  X(GET_FIELD)                                                                 \
  X(ILLEGAL)

class Runner {};

} // namespace Eval
