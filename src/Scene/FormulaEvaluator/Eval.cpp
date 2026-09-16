#include <Eval.hpp>
namespace Eval {

Value StringWrap::Call(const std::string &name,
                       const std::vector<Value> &args) {

  if (name == "podciąg") {
    if (args.size() < 1 || args.size() > 2)
      return Value{.err_msg = "Oczekiwano 1 lub 2 argumentów dla funkcji "
                              "podciąg() (początek, długość)"};

    const int *begin = std::get_if<int>(&args[0].data);
    const int *end = std::get_if<int>(&args[1].data);

    if (!begin && args.size() == 1 || (!begin || !end) && args.size() == 2)
      return Value{.err_msg =
                       "Funkcja podciąg() przyjmuje tylko liczby całkowite"};

    auto sw = std::make_shared<StringWrap>();
    sw.get()->data = data.substr(*begin, *end);
    return Value{.data = sw};

  } else if (name == "szukaj") {
    if (args.size() != 1)
      return Value{.err_msg = "Oczekiwano tylko 1 argumentu dla funkcji "
                              "szukaj() (szukany_ciąg)"};

    const auto *tobefound =
        std::get_if<std::shared_ptr<StringWrap>>(&args[0].data);

    if (!tobefound)
      return Value{.err_msg = "Funkcja szukaj() przyjmuje jedynie ciąg znaków"};

    std::string::size_type idx = data.find(tobefound->get()->data);
    if (idx == std::string::npos)
      return Value{.data = None{}};
    return Value{.data = (int)idx};

  } else if (name == "szukaj_od_prawej") {
    if (args.size() != 1)
      return Value{.err_msg = "Oczekiwano tylko 1 argumentu dla funkcji "
                              "szukaj_od_prawej() (szukany_ciąg)"};

    const auto *tobefound =
        std::get_if<std::shared_ptr<StringWrap>>(&args[0].data);

    if (!tobefound)
      return Value{
          .err_msg =
              "Funkcja szukaj_od_prawej() przyjmuje jedynie ciąg znaków"};

    std::string::size_type idx = data.rfind(tobefound->get()->data);
    if (idx == std::string::npos)
      return Value{.data = None{}};

    return Value{.data = (int)idx};

  } else if (name == "zawiera") {
    if (args.size() != 1)
      return Value{.err_msg = "Oczekiwano tylko 1 argumentu dla funkcji "
                              "zawiera() (szukany_ciąg)"};

    const auto *tobefound =
        std::get_if<std::shared_ptr<StringWrap>>(&args[0].data);

    if (!tobefound)
      return Value{.err_msg =
                       "Funkcja zawiera() przyjmuje jedynie ciąg znaków"};

    return Value{.data = data.contains(tobefound->get()->data)};
  }
};

} // namespace Eval
