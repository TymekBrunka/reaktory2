#include "Errors/Errors.hpp"
#include "FileUtils.hpp"
#include "Model.hpp"
#include <ResourceManager.hpp>

// ResourceManager::ResourceManager(const std::filesystem::path &folder_) {
// }

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

  Renderer::Model model = Renderer::Model::LoadFromFile(fs, filepath, true);

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
      Renderer::Model::LoadFromFile(FileUtils::RealFs{}, filepath, true);

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
