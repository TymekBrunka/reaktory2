#pragma once

#ifndef NDEBUG
#include <string>
#include <string_view>
#include <unordered_map>
#include <yyjson.h>
#else
#include <cstdint>
#endif

namespace Log {

#ifndef NDEBUG
struct string_hash {
  using is_transparent = void;
  [[nodiscard]] inline size_t operator()(const char *txt) const {
    return std::hash<std::string_view>{}(txt);
  }
  [[nodiscard]] inline size_t operator()(std::string_view txt) const {
    return std::hash<std::string_view>{}(txt);
  }
  [[nodiscard]] inline size_t operator()(const std::string &txt) const {
    return std::hash<std::string>{}(txt);
  }
};

struct translations_map_t {
  std::unordered_map<std::string, std::string, string_hash, std::equal_to<>>
      data;

  // inline const std::string &operator[](std::string_view key) {
  //   return (*data.find(key)).second;
  // }

  inline const std::string_view operator[](std::string_view key) {
    return (*data.find(key)).second;
  }
};
typedef std::string_view translations_key_t;
#define TRANSLATIONS_KEY(x) #x
#define TRANSLATIONS_INIT_LANG

#else
typedef std::string_view *tratranslations_map_t;

typedef uint32_t translations_key_t;
#define TRANSLATIONS_KEY(x) x
#endif

#ifndef TRANSLATIONS_DISABLE_SHORTHAND_MACROS
#define TL(x) TRANSLATIONS_KEY(x)
#endif

bool LoadTranslation(translations_map_t *translations,
                     const char *filepath);

} // namespace Log
