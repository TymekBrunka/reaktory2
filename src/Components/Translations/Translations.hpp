#pragma once
namespace Translations {

#ifndef NDEBUG
#include <unordered_map>
#include <yyjson.h>
struct translations_map_t {
  std::unordered_map<std::string_view, std::string_view> data;

  ~translations_map_t();
  translations_map_t(const translations_map_t &other) = delete;
  translations_map_t &operator=(const translations_map_t &other) = delete;
  translations_map_t(translations_map_t &&other);
  translations_map_t &operator=(translations_map_t &&other);
};
#define TRANSLATIONS_KEY(x) #x
#else
struct tratranslations_map_t {
  std::string_view *data;
};
#define TRANSLATIONS_KEY(x) x
#endif

#ifndef TRANSLATIONS_DISABLE_SHORTHAND_MACROS
#define T(x) TRANSLATIONS_KEY(x)
#endif

Load(const translations_map_t &translations, const char *filepath);

} // namespace Translations
