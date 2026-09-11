#include "Errors/Errors.hpp"
#include "FileUtils.hpp"
#include "Model.hpp"
#include "Renderer.hpp"
#include <ResourceManager.hpp>
#include <iostream>

// ResourceManager::ResourceManager(const std::filesystem::path &folder_) {
// }

static void
populateTexturesFromModel(Renderer::Model &model, std::string filename,
                          std::unordered_map<std::string, Renderer::rTexture2D,
                                             ResourceManager::string_hash,
                                             std::equal_to<>> &textures) {

  std::unordered_map<std::string, int, ResourceManager::string_hash,
                     std::equal_to<>>
      path2mesh_map;

  for (int i = 0; i < model.GetNumMeshes(); i++) {
    const std::string &path = model.GetTmpMaterialData(i)->diffuse1;
    if (!path.empty())
      if (path2mesh_map.find(path) == path2mesh_map.end())
        path2mesh_map[path] == i;
  }

  model.init();
  for (const auto &[name, idx] : path2mesh_map) {
    std::cerr << "populated texture " << name << "\n";
    textures[filename + "/" + name] = model.GetMaterials()[idx].diffuse1;
  }
}

Errors::Result<Renderer::Model *, int>
ResourceManager::ImportModel(const FileUtils::Fs &fs,
                             const std::filesystem::path &filepath,
                             bool allow_reload) {
  if (!std::filesystem::exists(filepath) ||
      !std::filesystem::is_regular_file(filepath))
    return Errors::Result<Renderer::Model *, int>::ERR(-2);

  std::string filename = filepath.filename().string();
  if (!allow_reload)
    if (models.find(filename) != models.end())
      return Errors::Result<Renderer::Model *, int>::OK(
          &(*models.find(filename)).second);

  Renderer::Model model = Renderer::Model::LoadFromFile(fs, filepath, false);
  populateTexturesFromModel(model, filename, textures);
  models[filename] = std::move(model);

  FileUtils::ReadResult blob = FileUtils::ReadFile(filepath).ok_unchecked();
  std::filesystem::create_directory(folder / filename);
  FileUtils::WriteFile(folder / filename / filename, blob.data, blob.length);

  return Errors::Result<Renderer::Model *, int>::OK(
      &(*models.find(filename)).second);
}

Errors::Result<Renderer::Model *, int>
ResourceManager::LoadModel(const char *name) {
  std::filesystem::path filepath = folder / name / name;

  if (!std::filesystem::exists(filepath) ||
      !std::filesystem::is_regular_file(filepath))
    return Errors::Result<Renderer::Model *, int>::ERR(-2);

  Renderer::Model model =
      Renderer::Model::LoadFromFile(FileUtils::RealFs{}, filepath, false);
  populateTexturesFromModel(model, name, textures);

  return Errors::Result<Renderer::Model *, int>::OK(
      &(*models.find(name)).second);
}

Errors::Result<Renderer::Model *, int>
ResourceManager::ImportModel(const FileUtils::Fs &fs,
                             const std::filesystem::path &filepath,
                             Renderer::Model *model, bool allow_reload) {

  std::string filename = filepath.filename().string();

  model->init();

  if (!allow_reload)
    if (models.find(filename) != models.end())
      return Errors::Result<Renderer::Model *, int>::OK(
          &(*models.find(filename)).second);

  models[filename] = std::move(*model);
  FileUtils::ReadResult blob = FileUtils::ReadFile(filepath).ok_unchecked();
  std::filesystem::create_directory(folder / filename);
  FileUtils::WriteFile(folder / filename / filename, blob.data, blob.length);

  return Errors::Result<Renderer::Model *, int>::OK(
      &(*models.find(filename)).second);
}
