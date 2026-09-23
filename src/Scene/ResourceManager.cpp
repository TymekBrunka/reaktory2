#include "FileUtils.hpp"
#include "Model.hpp"
#include "Renderer.hpp"
#include <ResourceManager.hpp>
#include <iostream>

// ResourceManager::ResourceManager(const std::filesystem::path &folder_) {
// }

static void populateTexturesFromModel(
    Renderer::Model &model, ManagedModel &mmodel, std::string filename,
    std::unordered_map<std::string, Renderer::rTexture2D,
                       ResourceManager::string_hash, std::equal_to<>>
        &textures) {

  std::unordered_map<std::string, int, ResourceManager::string_hash,
                     std::equal_to<>>
      path2mesh_map;

  for (int i = 0; i < model.GetNumMeshes(); i++) {
    const std::string path =
        filename + "/" + model.GetTmpMaterialData(i)->diffuse1;
    mmodel.materials.push_back(ManagedModel::ManagedMaterial{.diffuse1 = path});
    if (!model.GetTmpMaterialData(i)->diffuse1.empty())
      if (path2mesh_map.find(path) == path2mesh_map.end())
        path2mesh_map[path] == i;
  }

  model.init();
  for (const auto &[name, idx] : path2mesh_map) {
    std::cerr << "populated texture " << name << "\n";
    textures[name] = model.GetMaterials()[idx].diffuse1;
  }
}

std::expected<ManagedModel *, int>
ResourceManager::ImportModel(const FileUtils::Fs &fs,
                             const FileUtils::path &filepath,
                             bool allow_reload) {

  if (!fs.FileExists(filepath))
    return std::unexpected(-2);

  std::string filename = filepath.filename().to_string();
  if (!allow_reload)
    if (models.find(filename) != models.end())
      return (&(*models.find(filename)).second);

  auto model = Renderer::Model::LoadFromFile(fs, filepath, false);
  if (!model.has_value())
    return std::unexpected(3);

  ManagedModel mmodel{.model = std::move(model.value())};
  populateTexturesFromModel(mmodel.model, mmodel, filename, textures);

  models[filename] = std::move(mmodel);
  if (do_copy_files) {
    FileUtils::ReadResult blob = FileUtils::ReadFile(filepath).value();
    std::filesystem::create_directory(folder / filename);
    auto e1 = FileUtils::WriteFile(folder / "models" / filename / filename,
                                   blob.data, blob.length);
  }

  return (&(*models.find(filename)).second);
}

std::expected<ManagedModel *, int>
ResourceManager::ImportModel(const FileUtils::Fs &fs,
                             const FileUtils::path &filepath,
                             Renderer::Model *model, bool allow_reload) {

  std::string filename = filepath.filename().to_string();

  ManagedModel mmodel{.model = std::move(*model)};
  populateTexturesFromModel(mmodel.model, mmodel, filename, textures);

  if (!allow_reload)
    if (models.find(filename) != models.end())
      return (&(*models.find(filename)).second);

  models[filename] = std::move(mmodel);
  if (do_copy_files) {
    FileUtils::ReadResult blob = FileUtils::ReadFile(filepath).value();
    std::filesystem::create_directory(folder / "models" / filename);
    auto e1 = FileUtils::WriteFile(folder / "models" / filename / filename,
                                   blob.data, blob.length);
  }

  return (&(*models.find(filename)).second);
}

std::expected<Renderer::rTexture2D, int>
ResourceManager::ImportTexture(const FileUtils::Fs &fs,
                               const FileUtils::path &filepath,
                               bool allow_reload) {

  if (!fs.FileExists(filepath))
    return std::unexpected(-2);

  std::string filename = filepath.filename().to_string();
  if (!allow_reload)
    if (textures.find(filename) != textures.end())
      return ((*textures.find(filename)).second);

  auto res_fs = Renderer::Render::sLoadImage(fs, filepath);

  if (!res_fs.has_value()) {
    std::string path = filepath.to_string();
    switch (res_fs.error()) {
    case -2:
      Log::log(Log::ERROR | Log::SEV_MED, 0, "Resource manager",
               TL(MSG_GENERIC_FILE_NOT_FOUND), std::make_format_args(path));
      break;

    case -1:
      Log::log(Log::ERROR | Log::SEV_MED, 0, "Resource manager",
               TL(MSG_GENERIC_OPEN_ERROR), std::make_format_args(path));
      break;

    case 1:
      Log::log(Log::ERROR | Log::SEV_MED, 0, "Resource manager",
               TL(MSG_GENERIC_READ_ERROR), std::make_format_args(path));
      break;
    case 2:
      Log::log(Log::ERROR | Log::SEV_MED, 0, "Resource manager",
               TL(MSG_RENDER_LOAD_IMAGE_ERROR), std::make_format_args(path));
      break;
    default:
      break;
    }
    return std::unexpected(res_fs.error());
  }

  auto res_img = Renderer::Render::sLoadTexture(res_fs.value());
  free(res_fs.value().pixels);

  if (!res_img.has_value()) {
    std::string path = filepath.to_string();
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Resource manager",
             TL(MSG_RENDER_LOAD_IMAGE_ERROR), std::make_format_args(path));
    return std::unexpected(3);
  }

  textures[filename] = res_img.value();

  if (do_copy_files) {
    FileUtils::ReadResult blob = FileUtils::ReadFile(filepath).value();
    std::filesystem::create_directory(folder / "textures");
    auto e1 = FileUtils::WriteFile(folder / "textures" / filename, blob.data,
                                   blob.length);
  }

  return (res_img.value());
}

std::expected<Renderer::rTexture2D, int>
ResourceManager::ImportTexture(const FileUtils::Fs &fs,
                               const FileUtils::path &filepath,
                               Renderer::Image *image, bool allow_reload) {

  if (!fs.FileExists(filepath)) {
    free(image->pixels);
    return std::unexpected(-2);
  }

  std::string filename = filepath.filename().to_string();
  if (!allow_reload)
    if (textures.find(filename) != textures.end()) {
      free(image->pixels);
      return ((*textures.find(filename)).second);
    }

  std::expected<Renderer::rTexture2D, bool> res_img =
      Renderer::Render::sLoadTexture(*image);
  free(image->pixels);

  if (!res_img.has_value()) {
    std::string path = filepath.to_string();
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Resource manager",
             TL(MSG_RENDER_LOAD_IMAGE_ERROR), std::make_format_args(path));
    return std::unexpected(3);
  }

  textures[filename] = res_img.value();

  if (do_copy_files) {
    FileUtils::ReadResult blob = FileUtils::ReadFile(filepath).value();
    std::filesystem::create_directory(folder / "textures");
    auto e1 = FileUtils::WriteFile(folder / "textures" / filename, blob.data,
                                   blob.length);
  }

  return res_img.value();
}
