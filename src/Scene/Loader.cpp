#include "FileUtils.hpp"
#include "ObjectPool.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <Scene.hpp>
#include <cstring>
#include <iostream>
#include <yyjson.h>

#define json_assert(assert_)                                                   \
  if (!(assert_)) {                                                            \
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",                            \
             TL(MSG_LOADING_JSON_ERROR), std::make_format_args(#assert_));     \
    yyjson_doc_free(doc);                                                      \
    return false;                                                              \
  }

bool Scene::load() {
  auto res_fs = FileUtils::ReadFile(current_folder / "scene.json");

  if (!res_fs.has_value()) {
    std::string path = (current_folder / "scene.json").string();
    switch (res_fs.error()) {
    case -2:
      Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
               TL(MSG_GENERIC_FILE_NOT_FOUND), std::make_format_args(path));
      return false;

    case -1:
      Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
               TL(MSG_GENERIC_OPEN_ERROR), std::make_format_args(path));
      return false;

    case 1:
      Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
               TL(MSG_GENERIC_READ_ERROR), std::make_format_args(path));
      return false;

    default:
      return false;
    }
  }

  yyjson_doc *doc = yyjson_read(res_fs.value().data, res_fs.value().length,
                                YYJSON_READ_ALLOW_COMMENTS |
                                    YYJSON_READ_ALLOW_TRAILING_COMMAS);

  yyjson_val *root = yyjson_doc_get_root(doc);
  json_assert(yyjson_is_obj(root));

  yyjson_val *format_version_json = yyjson_obj_get(root, "format-version");
  json_assert(format_version_json != NULL);
  json_assert(yyjson_is_num(format_version_json));
  int format_version = yyjson_get_int(format_version_json);

  yyjson_val *app_version_json = yyjson_obj_get(root, "app-version");
  json_assert(app_version_json != NULL);
  json_assert(yyjson_is_num(app_version_json));
  int app_version = yyjson_get_int(app_version_json);

  yyjson_val *selected_sheet_json = yyjson_obj_get(root, "selected-sheet");
  json_assert(selected_sheet_json != NULL);
  json_assert(yyjson_is_str(selected_sheet_json));
  selected_sheet = yyjson_get_str(selected_sheet_json);

  yyjson_val *objects = yyjson_obj_get(root, "objects");
  json_assert(objects != NULL);
  json_assert(yyjson_is_arr(objects));

  // populate object pool
  std::cerr << "Objects array size is: " << yyjson_arr_size(objects) << "\n";
  for (int i = 0; i < (int)yyjson_arr_size(objects) - 1; i++)
    objPool.add_node(objPool.rootH());

  int16_t objidx = 0;
  yyjson_arr_iter objects_iter = yyjson_arr_iter_with(objects);
  while (yyjson_val *object = yyjson_arr_iter_next(&objects_iter)) {
    json_assert(yyjson_is_obj(object));
    Object &node = objPool.get(objPool.rootH())[objidx];

    yyjson_val *obj_name = yyjson_obj_get(object, "name");
    json_assert(obj_name != NULL);
    json_assert(yyjson_is_str(obj_name));
    node.name = yyjson_get_str(obj_name);

    yyjson_val *obj_collapsed = yyjson_obj_get(object, "collapsed");
    json_assert(obj_collapsed != NULL);
    json_assert(yyjson_is_bool(obj_collapsed));
    node.collapsed = yyjson_get_bool(obj_collapsed);

    if (yyjson_val *obj_script_class = yyjson_obj_get(object, "script-class")) {
      json_assert(yyjson_is_int(obj_script_class));
      node.script_class_idx = yyjson_get_int(obj_script_class);
    }

    yyjson_val *obj_variant = yyjson_obj_get(object, "variant");
    json_assert(obj_variant != NULL);

    json_assert(yyjson_is_str(obj_variant) || yyjson_is_arr(obj_variant));
    if (yyjson_is_str(obj_variant)) {
      if (yyjson_equals_str(obj_variant, "NODE"))
        node.variant = oNode{};
      else {
        const char *obj_variant_name = NULL;
        json_assert(obj_variant_name == "NODE");
      }

    } else if (yyjson_is_arr(obj_variant)) {
      int obj_variant_arr_number_of_elements = yyjson_arr_size(obj_variant);
      json_assert(obj_variant_arr_number_of_elements > 0);
      yyjson_val *obj_variant_name_json = yyjson_arr_get(obj_variant, 0);
      json_assert(yyjson_is_str(obj_variant_name_json));
      // const char *obj_variant_name = yyjson_get_str(obj_variant_name_json);

      if (yyjson_equals_str(obj_variant_name_json, "MODEL")) {
        const char *obj_variant_name = "MODEL";
        json_assert(obj_variant_name == "MODEL" &&
                    obj_variant_arr_number_of_elements == 6);

        oModel model;
        yyjson_val *model_name = yyjson_arr_get(obj_variant, 1);
        json_assert(model_name != NULL);
        json_assert(yyjson_is_str(model_name));
        model.model = yyjson_get_str(model_name);

        yyjson_val *model_animation_idx = yyjson_arr_get(obj_variant, 2);
        json_assert(model_animation_idx != NULL);
        json_assert(yyjson_is_int(model_animation_idx));
        int model_animation_idx_ = yyjson_get_int(model_animation_idx);
        json_assert(model_animation_idx_ >= -1);
        model.animation_idx = model_animation_idx_;

        yyjson_val *model_animation_time = yyjson_arr_get(obj_variant, 3);
        json_assert(model_animation_time != NULL);
        json_assert(yyjson_is_num(model_animation_time));
        model.animation_time = yyjson_get_real(model_animation_time);

        yyjson_val *model_animation_speed = yyjson_arr_get(obj_variant, 4);
        json_assert(model_animation_speed != NULL);
        json_assert(yyjson_is_num(model_animation_speed));
        model.animation_speed = yyjson_get_real(model_animation_speed);

        yyjson_val *model_materials = yyjson_arr_get(obj_variant, 5);
        json_assert(model_materials != NULL);
        json_assert(yyjson_is_arr(model_materials));

        yyjson_arr_iter materials_iter = yyjson_arr_iter_with(model_materials);
        while (yyjson_val *model_material =
                   yyjson_arr_iter_next(&materials_iter)) {
          json_assert(yyjson_is_arr(model_material));
          int model_material_number_of_elements =
              yyjson_arr_size(model_material);
          json_assert(model_material_number_of_elements == 2);

          Material material;
          yyjson_val *model_material_texture =
              yyjson_arr_get(model_material, 0);
          json_assert(yyjson_is_str(model_material_texture));
          material.diffuse1 = yyjson_get_str(model_material_texture);

          yyjson_val *model_material_color = yyjson_arr_get(model_material, 1);
          json_assert(yyjson_is_arr(model_material_color));
          int model_material_color_number_of_elements =
              yyjson_arr_size(model_material_color);
          json_assert(model_material_color_number_of_elements == 4);
          for (int i = 0; i < 4; i++) {
            yyjson_val *material_color_element =
                yyjson_arr_get(model_material_color, i);
            json_assert(yyjson_is_num(material_color_element));
            glm::value_ptr(material.color_diffuse)[i] =
                yyjson_get_real(material_color_element);
          }

          model.materials.push_back(material);
        }

        node.variant = model;

      } else if (yyjson_equals_str(obj_variant_name_json, "DATAPOINT")) {
      } else if (yyjson_equals_str(obj_variant_name_json, "TEXT3D")) {
      } else {
        const char *obj_variant_name = NULL;
        json_assert(obj_variant_name == "MODEL" ||
                    obj_variant_name == "DATAPOINT" ||
                    obj_variant_name == "TEXT3D");
      }
    }

    objidx++;
  }

  return true;
}
