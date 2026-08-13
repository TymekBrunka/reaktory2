#include <Translations.hpp>
#ifndef NDEBUG

~Translations::translations_map_t::translations_map_t() {
  if (json)
    yyjson_doc_free(json);
}

translations_map_t::tratranslations_map_t(translations_map_t &&other) {
  json = other.json;
  other.json = nullptr;
};

translations_map_t &
tratranslations_map_t::operator=(translations_map_t &&other) {
  if (this != &other) {
    json = other.json;
    other.json = nullptr;
  }
  return *this;
};

#define json_assert(x)                                                         \
  if (!(x)) {                                                                  \
    yyjson_doc_free(doc);                                                      \
    return false;                                                              \
  }

bool Translations::Load(translations_map_t *translations,
                        const char *filepath) {

  yyjson_read_err err;
  yyjson_doc *doc = yyjson_read_file(
      filepath, YYJSON_READ_ALLOW_COMMENTS | YYJSON_READ_ALLOW_TRAILING_COMMAS,
      0, &err);

  if (!doc)
    return false;

  yyjson_val *root_val = yyjson_doc_get_root(doc);
  json_assert(root_val != nullptr);
  json_assert(yyjson_is_obj(root_val));

  yyjson_val *key, *val;
  yyjson_obj_iter iter = yyjson_obj_iter_with(root_val);
  while ((key = yyjson_obj_iter_next(&iter))) {
    val = yyjson_obj_iter_get_val(key);
    json_assert(yyjson_is_arr(val));

    yyjson_val *el;
    yyjson_arr_iter iter = yyjson_arr_iter_with(val);
    while ((el = yyjson_arr_iter_next(&iter))) {
    }
  }

  return true;
}
#else

bool Translations::Load(const translations_map_t &translations,
                        const char *filepath) {
  return true;
}
#endif
