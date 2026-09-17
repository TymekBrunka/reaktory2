#include "Errors/Errors.hpp"
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
    const std::string path = filename + "/" + model.GetTmpMaterialData(i)->diffuse1;
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

Errors::Result<ManagedModel *, int>
ResourceManager::ImportModel(const FileUtils::Fs &fs,
                             const FileUtils::path &filepath,
                             bool allow_reload) {

  if (!fs.FileExists(filepath))
    return Errors::Result<ManagedModel *, int>::ERR(-2);

  std::string filename = filepath.filename().to_string();
  if (!allow_reload)
    if (models.find(filename) != models.end())
      return Errors::Result<ManagedModel *, int>::OK(
          &(*models.find(filename)).second);

  Renderer::Model model = Renderer::Model::LoadFromFile(fs, filepath, false);
  ManagedModel mmodel{.model = std::move(model)};
  populateTexturesFromModel(mmodel.model, mmodel, filename, textures);

  models[filename] = std::move(mmodel);
  if (do_copy_files) {
    FileUtils::ReadResult blob = FileUtils::ReadFile(filepath).ok_unchecked();
    std::filesystem::create_directory(folder / filename);
    FileUtils::WriteFile(folder / "models" / filename / filename, blob.data,
                         blob.length);
  }

  return Errors::Result<ManagedModel *, int>::OK(
      &(*models.find(filename)).second);
}

Errors::Result<ManagedModel *, int>
ResourceManager::ImportModel(const FileUtils::Fs &fs,
                             const FileUtils::path &filepath,
                             Renderer::Model *model, bool allow_reload) {

  std::string filename = filepath.filename().to_string();

  ManagedModel mmodel{.model = std::move(*model)};
  populateTexturesFromModel(mmodel.model, mmodel, filename, textures);

  if (!allow_reload)
    if (models.find(filename) != models.end())
      return Errors::Result<ManagedModel *, int>::OK(
          &(*models.find(filename)).second);

  models[filename] = std::move(mmodel);
  if (do_copy_files) {
    FileUtils::ReadResult blob = FileUtils::ReadFile(filepath).ok_unchecked();
    std::filesystem::create_directory(folder / "models" / filename);
    FileUtils::WriteFile(folder / "models" / filename / filename, blob.data,
                         blob.length);
  }

  return Errors::Result<ManagedModel *, int>::OK(
      &(*models.find(filename)).second);
}

Errors::Result<Renderer::rTexture2D, int>
ResourceManager::ImportTexture(const FileUtils::Fs &fs,
                               const FileUtils::path &filepath,
                               bool allow_reload) {

  if (!fs.FileExists(filepath))
    return Errors::Result<Renderer::rTexture2D, int>::ERR(-2);

  std::string filename = filepath.filename().to_string();
  if (!allow_reload)
    if (textures.find(filename) != textures.end())
      return Errors::Result<Renderer::rTexture2D, int>::OK(
          (*textures.find(filename)).second);

  Errors::Result<Renderer::Image, int> res_fs =
      Renderer::Render::sLoadImage(fs, filepath);

  if (!res_fs.is_ok) {
    std::string path = filepath.to_string();
    switch (res_fs.value.error) {
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
    return Errors::Result<Renderer::rTexture2D, int>::ERR(res_fs.value.error);
  }

  Errors::Result<Renderer::rTexture2D, Errors::no_error> res_img =
      Renderer::Render::sLoadTexture(res_fs.ok_unchecked());
  free(res_fs.ok_unchecked().pixels);

  if (!res_img.is_ok) {
    std::string path = filepath.to_string();
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Resource manager",
             TL(MSG_RENDER_LOAD_IMAGE_ERROR), std::make_format_args(path));
    return Errors::Result<Renderer::rTexture2D, int>::ERR(3);
  }

  textures[filename] = res_img.ok_unchecked();

  if (do_copy_files) {
    FileUtils::ReadResult blob = FileUtils::ReadFile(filepath).ok_unchecked();
    std::filesystem::create_directory(folder / "textures");
    FileUtils::WriteFile(folder / "textures" / filename, blob.data,
                         blob.length);
  }

  return Errors::Result<Renderer::rTexture2D, int>::OK(res_img.ok_unchecked());
}

Errors::Result<Renderer::rTexture2D, int>
ResourceManager::ImportTexture(const FileUtils::Fs &fs,
                               const FileUtils::path &filepath,
                               Renderer::Image *image, bool allow_reload) {

  if (!fs.FileExists(filepath)) {
    free(image->pixels);
    return Errors::Result<Renderer::rTexture2D, int>::ERR(-2);
  }

  std::string filename = filepath.filename().to_string();
  if (!allow_reload)
    if (textures.find(filename) != textures.end()) {
      free(image->pixels);
      return Errors::Result<Renderer::rTexture2D, int>::OK(
          (*textures.find(filename)).second);
    }

  Errors::Result<Renderer::rTexture2D, Errors::no_error> res_img =
      Renderer::Render::sLoadTexture(*image);
  free(image->pixels);

  if (!res_img.is_ok) {
    std::string path = filepath.to_string();
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Resource manager",
             TL(MSG_RENDER_LOAD_IMAGE_ERROR), std::make_format_args(path));
    return Errors::Result<Renderer::rTexture2D, int>::ERR(3);
  }

  textures[filename] = res_img.ok_unchecked();

  if (do_copy_files) {
    FileUtils::ReadResult blob = FileUtils::ReadFile(filepath).ok_unchecked();
    std::filesystem::create_directory(folder / "textures");
    FileUtils::WriteFile(folder / "textures" / filename, blob.data,
                         blob.length);
  }

  return Errors::Result<Renderer::rTexture2D, int>::OK(res_img.ok_unchecked());
}
