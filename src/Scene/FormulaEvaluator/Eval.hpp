#pragma once
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
  inline Value Call(const std::string &name,
                    const std::vector<Value> &args) override;
};

struct None {
  bool dummy = false;
};

struct Value {
  std::string err_msg;
  std::variant<None, void *, StringWrap, Class *, int, float, bool,
               std::vector<Value>>
      data;
};

inline Value StringWrap::Get(const std::string &name) {
  if (name == "długość") {
    return Value{.data = (int)data.size()};
  }
}

inline Value StringWrap::Call(const std::string &name,
                              const std::vector<Value> &args) {

  if (name == "podciąg") {
    if (args.size() < 1 || args.size() > 2)
      return Value{.err_msg = "Oczekiwano 2 argumentów dla funkcji podciąg()"};

    const int *begin = std::get_if<int>(&args[0]);
    const int *end = std::get_if<int>(&args[1]);

    if (!begin || !end)
      return Value{.err_msg = "Funkcja podciąg() przyjmuje tylko liczby całkowite"};

    return Value {
      .data = StringWrap { .data = data.substr(*begin, *end) }
    }
  }
};

#define EVAL_ast_tag_names                                                     \
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
  ASTtag tag;
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
