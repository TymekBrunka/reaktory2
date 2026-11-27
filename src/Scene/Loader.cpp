#include "FileUtils.hpp"
#include "ObjectPool.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <Scene.hpp>
#include <cstring>
#include <iostream>
#include <vector>
#include <yyjson.h>

#include "imgui.h"

#include "ImGuizmo.h"

#define json_assert(assert_)                                                   \
  if (!(assert_)) {                                                            \
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",                            \
             TL(MSG_LOADING_JSON_ERROR), std::make_format_args(#assert_));     \
    yyjson_doc_free(doc);                                                      \
    return false;                                                              \
  }

bool Scene::load_script_params(void *obj_script_params_iter_, void *doc_,
                               std::vector<LuaParam> &params) {
  auto doc = (yyjson_doc *)doc_;
  auto obj_script_params_iter = (yyjson_arr_iter *)obj_script_params_iter_;

  int param_idx = 0;
  while (yyjson_val *script_param =
             yyjson_arr_iter_next(obj_script_params_iter)) {

    LuaParam param;
    if (yyjson_is_null(script_param)) {
      param = Nil{};

    } else if (yyjson_is_int(script_param)) {
      param = (int)yyjson_get_int(script_param);

    } else if (yyjson_is_real(script_param)) {
      param = (float)yyjson_get_real(script_param);

    } else if (yyjson_is_str(script_param)) {
      param = std::string(yyjson_get_str(script_param));

    } else if (yyjson_is_arr(script_param)) {

      int script_param_number_of_elements = yyjson_arr_size(script_param);
      json_assert(script_param_number_of_elements > 1);
      yyjson_val *script_param_type = yyjson_arr_get(script_param, 0);
      json_assert(yyjson_is_str(script_param_type));

      if (yyjson_equals_str(script_param_type, "MATERIAL")) {
        const char *script_param_type_ = "MATERIAL";
        json_assert(script_param_type_ == "MATERIAL" &&
                    script_param_number_of_elements == 3);

        Material material;
        yyjson_val *script_param_material_texture =
            yyjson_arr_get(script_param, 1);
        json_assert(yyjson_is_str(script_param_material_texture));
        material.diffuse1 = yyjson_get_str(script_param_material_texture);

        yyjson_val *script_param_material_color =
            yyjson_arr_get(script_param, 2);
        json_assert(yyjson_is_arr(script_param_material_color));
        int script_param_material_color_number_of_elements =
            yyjson_arr_size(script_param_material_color);
        json_assert(script_param_material_color_number_of_elements == 4);

        for (int i = 0; i < 4; i++) {
          yyjson_val *material_color_element =
              yyjson_arr_get(script_param_material_color, i);
          json_assert(yyjson_is_num(material_color_element));
          glm::value_ptr(material.color_diffuse)[i] =
              yyjson_get_real(material_color_element);
        }

        param = material;

      } else if (yyjson_equals_str(script_param_type, "OBJH")) {
        const char *script_param_type_ = "OBJH";
        json_assert(script_param_type_ == "OBJH" &&
                    script_param_number_of_elements == 2);

        objH objh;
        yyjson_val *script_param_objh = yyjson_arr_get(script_param, 1);
        json_assert(yyjson_is_int(script_param_objh));
        int16_t script_param_objh_ = yyjson_get_int(script_param_objh);
        objh.idx = script_param_objh_;
        json_assert(script_param_objh_ == -1 ||
                    script_param_number_of_elements < objPool.real_size());

        if (script_param_objh_ >= 0) {
          Object &object = objPool.get(objPool.rootH())[script_param_objh_];
          bool object_ = !(std::get_if<oEmptySlot>(&object.variant));
          json_assert(script_param_objh_ && object_ != NULL);
          objh.gen = object.gen;
        }

        param = objh;

      } else if (yyjson_equals_str(script_param_type, "MODEL")) {
        const char *script_param_type_ = "MODEL";
        json_assert(script_param_type_ == "MODEL" &&
                    script_param_number_of_elements == 2);

        yyjson_val *script_param_model = yyjson_arr_get(script_param, 1);
        json_assert(yyjson_is_str(script_param_model));
        param = _ModelName{.model = yyjson_get_str(script_param_model)};

      } else if (yyjson_equals_str(script_param_type, "HEADER")) {
        const char *script_param_type_ = "HEADER";
        json_assert(script_param_type_ == "HEADER" &&
                    script_param_number_of_elements == 2);

        yyjson_val *script_param_header = yyjson_arr_get(script_param, 1);
        json_assert(yyjson_is_str(script_param_header));
        param = _HeaderName{.header = yyjson_get_str(script_param_header)};
      }
    } else {
      const char *script_param_type_ = nullptr;
      json_assert(
          script_param_type_ == "MATERIAL" || script_param_type_ == "OBJH" ||
          script_param_type_ == "MODEL" || script_param_type_ == "HEADER");
    }

    params[param_idx] = param;
    param_idx++;
  }
  return true;
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

  if (yyjson_val *sheet2sampler = yyjson_obj_get(root, "sheet2sampler")) {
    json_assert(yyjson_is_obj(sheet2sampler));

    yyjson_obj_iter sheet2sampler_iter = yyjson_obj_iter_with(sheet2sampler);
    while (yyjson_val *sheet_name = yyjson_obj_iter_next(&sheet2sampler_iter)) {
      json_assert(yyjson_is_str(sheet_name));
      yyjson_val *sheet_json = yyjson_obj_iter_get_val(sheet_name);
      json_assert(yyjson_is_obj(sheet_json));

      auto it = sheets.find(yyjson_get_str(sheet_name));
      if (it == sheets.end())
        continue;
      Sheet &sheet = it->second;

      yyjson_obj_iter sheet_json_iter = yyjson_obj_iter_with(sheet_json);
      while (yyjson_val *sheet_header_name =
                 yyjson_obj_iter_next(&sheet_json_iter)) {

        json_assert(yyjson_is_str(sheet_header_name));
        yyjson_val *sheet_sampleridx =
            yyjson_obj_iter_get_val(sheet_header_name);
        json_assert(yyjson_is_int(sheet_sampleridx));

        int j = 0;
        for (const auto &header : sheet.headers) {
          if (header == yyjson_get_str(sheet_header_name)) {
            sheet.sheet2sampler[j] = yyjson_get_int(sheet_sampleridx);
          }
          j++;
        }
      }
    }
  }

  yyjson_val *objects = yyjson_obj_get(root, "objects");
  json_assert(objects != NULL);
  json_assert(yyjson_is_arr(objects));

  // populate object pool
  std::cerr << "Objects array size is: " << yyjson_arr_size(objects) << "\n";
  for (int i = 0; i < (int)yyjson_arr_size(objects) - 1; i++)
    objPool.add(Object{});

  if (yyjson_val *prefabs_json = yyjson_obj_get(root, "prefabs")) {
    json_assert(yyjson_is_arr(prefabs_json));

    yyjson_arr_iter prefabs_iter = yyjson_arr_iter_with(prefabs_json);
    while (yyjson_val *prefab = yyjson_arr_iter_next(&prefabs_iter)) {
      json_assert(yyjson_is_int(prefab));
      int16_t prefab_obj = yyjson_get_int(prefab);
      json_assert(prefab_obj > 0 && prefab_obj < objPool.real_size());
      prefabs.push_back(Scene::Prefab{
          .gen = 1,
          .obj =
              objPool.get(objPool.rootH())[prefab_obj].toHandle(prefab_obj)});
    }
  }

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
      node.change_script_class(&objPool, yyjson_get_int(obj_script_class));
    }

    if (yyjson_val *obj_children = yyjson_obj_get(object, "children")) {
      json_assert(yyjson_is_arr(obj_children));

      yyjson_arr_iter obj_children_iter = yyjson_arr_iter_with(obj_children);
      while (yyjson_val *obj_child = yyjson_arr_iter_next(&obj_children_iter)) {
        json_assert(yyjson_is_int(obj_child));
        int obj_child_idx = yyjson_get_int(obj_child);
        json_assert(obj_child_idx > 0 && obj_child_idx < objPool.real_size());

        node.children.push_back((int16_t)obj_child_idx);
        Object &node_ = objPool.get(objPool.rootH())[obj_child_idx];
        node_.parent = node.toHandle(objidx);
      }
    }

    if (yyjson_val *obj_translation_json =
            yyjson_obj_get(object, "translation")) {

      json_assert(yyjson_is_arr(obj_translation_json));
      json_assert(yyjson_arr_size(obj_translation_json) == 3);
      float obj_translation[3] = {0};
      for (int i = 0; i < 3; i++) {
        yyjson_val *translation_element =
            yyjson_arr_get(obj_translation_json, i);
        json_assert(yyjson_is_num(translation_element));
        obj_translation[i] = yyjson_get_real(translation_element);
      }

      yyjson_val *obj_rotation_json = yyjson_obj_get(object, "rotation");
      json_assert(obj_rotation_json != NULL);
      json_assert(yyjson_is_arr(obj_rotation_json));
      json_assert(yyjson_arr_size(obj_rotation_json) == 3);
      float obj_rotation[3] = {0};
      for (int i = 0; i < 3; i++) {
        yyjson_val *rotation_element = yyjson_arr_get(obj_rotation_json, i);
        json_assert(yyjson_is_num(rotation_element));
        obj_rotation[i] = yyjson_get_real(rotation_element);
      }

      yyjson_val *obj_scale_json = yyjson_obj_get(object, "scale");
      json_assert(obj_scale_json != NULL);
      json_assert(yyjson_is_arr(obj_scale_json));
      json_assert(yyjson_arr_size(obj_scale_json) == 3);
      float obj_scale[3] = {0};
      for (int i = 0; i < 3; i++) {
        yyjson_val *scale_element = yyjson_arr_get(obj_scale_json, i);
        json_assert(yyjson_is_num(scale_element));
        obj_scale[i] = yyjson_get_real(scale_element);
      }

      ImGuizmo::RecomposeMatrixFromComponents(obj_translation, obj_rotation,
                                              obj_scale,
                                              glm::value_ptr(node.transform));
    }

    if (yyjson_val *obj_script_params =
            yyjson_obj_get(object, "script-params")) {
      json_assert(yyjson_is_arr(obj_script_params));

      yyjson_arr_iter obj_script_params_iter =
          yyjson_arr_iter_with(obj_script_params);
      if (!load_script_params(&obj_script_params_iter, doc, node.lua_params))
        return false;
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
        const char *obj_variant_name = "DATAPOINT";
        json_assert(obj_variant_name == "DATAPOINT" &&
                    obj_variant_arr_number_of_elements == 2);

        oDataPoint2 datapoint;
        yyjson_val *datapoint_headers = yyjson_arr_get(obj_variant, 1);
        json_assert(yyjson_is_arr(datapoint_headers));
        yyjson_arr_iter datapoint_headers_iter =
            yyjson_arr_iter_with(datapoint_headers);

        while (yyjson_val *datapoint_header =
                   yyjson_arr_iter_next(&datapoint_headers_iter)) {
          json_assert(yyjson_is_arr(datapoint_header));
          int datapoint_header_number_of_elements =
              yyjson_arr_size(datapoint_header);
          json_assert(datapoint_header_number_of_elements == 3);

          DPHeader header;
          yyjson_val *datapoint_header_name =
              yyjson_arr_get(datapoint_header, 0);
          json_assert(yyjson_is_str(datapoint_header_name));
          header.header = yyjson_get_str(datapoint_header_name);

          yyjson_val *datapoint_header_min =
              yyjson_arr_get(datapoint_header, 1);
          json_assert(yyjson_is_num(datapoint_header_min));
          header.min = yyjson_get_real(datapoint_header_min);

          yyjson_val *datapoint_header_max =
              yyjson_arr_get(datapoint_header, 2);
          json_assert(yyjson_is_num(datapoint_header_max));
          header.max = yyjson_get_real(datapoint_header_max);

          datapoint.headers.push_back(std::move(header));
        }

        node.variant = datapoint;

      } else if (yyjson_equals_str(obj_variant_name_json, "TEXT3D")) {
        const char *obj_variant_name = "TEXT3D";
        json_assert(obj_variant_name == "TEXT3D" &&
                    obj_variant_arr_number_of_elements == 2);

        yyjson_val *text3d_text = yyjson_arr_get(obj_variant, 1);
        json_assert(yyjson_is_str(text3d_text));
        Text3D text3d;
        text3d.setText(yyjson_get_str(text3d_text));

        node.variant = text3d;

      } else if (yyjson_equals_str(obj_variant_name_json, "PREFAB")) {
        const char *obj_variant_name = "PREFAB";
        json_assert(obj_variant_name == "PREFAB" &&
                    (obj_variant_arr_number_of_elements == 2 ||
                     obj_variant_arr_number_of_elements == 3));

        yyjson_val *obj_prefab = yyjson_arr_get(obj_variant, 1);
        json_assert(yyjson_is_int(obj_prefab));
        int obj_prefab_idx = yyjson_get_int(obj_prefab);
        json_assert(obj_prefab_idx == -1 || (obj_prefab_idx >= 0 && obj_prefab_idx < prefabs.size()));

        oPrefab prefab{.initialised = false,
                       .idx = obj_prefab_idx == -1 ? -1 : obj_prefab_idx + 1};

        if (obj_variant_arr_number_of_elements == 3) {
          yyjson_val *prefab_script_params = yyjson_arr_get(obj_variant, 2);
          json_assert(yyjson_is_arr(prefab_script_params));

          prefab.prefab_params.resize(yyjson_arr_size(prefab_script_params));
          yyjson_arr_iter prefab_script_params_iter =
              yyjson_arr_iter_with(prefab_script_params);
          if (!load_script_params(&prefab_script_params_iter, doc,
                                  prefab.prefab_params))
            return false;
        }

        node.variant = std::move(prefab);

      } else {
        const char *obj_variant_name = NULL;
        json_assert(
            yyjson_is_arr(obj_variant) &&
            (obj_variant_name == "MODEL" || obj_variant_name == "DATAPOINT" ||
             obj_variant_name == "TEXT3D" || obj_variant_name == "PREFAB"));
      }
    }

    objidx++;
  }

  return true;
}
