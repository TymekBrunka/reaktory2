#include "Errors/Errors.hpp"
#include "FileUtils.hpp"
#include "Model.hpp"
#include <ModelManager.hpp>

ModelManager::ModelManager(const std::filesystem::path &folder_) {
  folder = folder_;
}

Errors::Result<Renderer::Model *, int>
ModelManager::Import(const std::filesystem::path &filepath) {
  if (!std::filesystem::exists(filepath))
    return Errors::Result<Renderer::Model *, int>::ERR(-2);

  Renderer::Model model =
      Renderer::Model::LoadFromFile(filepath.string().c_str(), true);

  std::string filename = filepath.filename().string();
  models[filename] = std::move(model);

  FileUtils::ReadResult blob = FileUtils::ReadFile(filepath).ok_unchecked();
  FileUtils::WriteFile(folder / filename, blob.data, blob.length);

  return Errors::Result<Renderer::Model *, int>::OK(
      &(*models.find(filename)).second);
}

Errors::Result<Renderer::Model *, int> ModelManager::Load(const char *name) {
  std::filesystem::path filepath = folder / name;

  if (!std::filesystem::exists(filepath))
    return Errors::Result<Renderer::Model *, int>::ERR(-2);

  Renderer::Model model =
      Renderer::Model::LoadFromFile(filepath.string().c_str(), true);

  return Errors::Result<Renderer::Model *, int>::OK(
      &(*models.find(name)).second);
}

Errors::Result<Renderer::Model *, int>
ModelManager::Import(const std::filesystem::path &filepath,
                     Renderer::Model *model) {

  std::string filename = filepath.filename().string();

  model->init();
  models[filename] = std::move(*model);
  FileUtils::ReadResult blob = FileUtils::ReadFile(filepath).ok_unchecked();
  FileUtils::WriteFile(folder / filename, blob.data, blob.length);

  return Errors::Result<Renderer::Model *, int>::OK(
      &(*models.find(filename)).second);
}
