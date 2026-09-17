#pragma once
#include <Errors/Errors.hpp>
#include <FileUtils.hpp>
#include <Model.hpp>
#include <Renderer.hpp>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

struct ManagedModel {
  struct ManagedMaterial {
    std::string diffuse1;
  };

  std::vector<ManagedMaterial> materials;
  Renderer::Model model;
};

class ResourceManager {
public:
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

  bool do_copy_files = true;
private:
  std::filesystem::path folder{};

  std::unordered_map<std::string, ManagedModel, string_hash, std::equal_to<>>
      models;

  std::unordered_map<std::string, Renderer::rTexture2D, string_hash,
                     std::equal_to<>>
      textures;

public:
  ResourceManager() = default;
  ResourceManager(const std::filesystem::path &folder_) : folder(folder_) {};
  ~ResourceManager() = default;
  ResourceManager(const ResourceManager &other) = delete;
  ResourceManager &operator=(const ResourceManager &other) = delete;
  ResourceManager(ResourceManager &&other) = default;
  ResourceManager &operator=(ResourceManager &&other) = default;

  inline const decltype(models) &GetModelsMap() const { return models; }
  inline const decltype(textures) &GetTexturesMap() const { return textures; }

  inline ManagedModel *GetModel(const char *txt) {
    auto iter = models.find(txt);
    return iter != models.end() ? &(*iter).second : nullptr;
  }

  inline ManagedModel *GetModel(std::string_view txt) {
    auto iter = models.find(txt);
    return iter != models.end() ? &(*iter).second : nullptr;
  }

  inline ManagedModel *GetModel(const std::string &txt) {
    auto iter = models.find(txt);
    return iter != models.end() ? &(*iter).second : nullptr;
  }

  inline int GetTexture(const char *txt) {
    auto iter = textures.find(txt);
    return iter != textures.end() ? (*iter).second : -1;
  }

  inline int GetTexture(std::string_view txt) {
    auto iter = textures.find(txt);
    return iter != textures.end() ? (*iter).second : -1;
  }

  inline int GetTexture(const std::string &txt) {
    auto iter = textures.find(txt);
    return iter != textures.end() ? (*iter).second : -1;
  }

  Errors::Result<ManagedModel *, int>
  ImportModel(const FileUtils::Fs &fs, const FileUtils::path &filepath,
              bool allow_reload = false);

  Errors::Result<ManagedModel *, int>
  ImportModel(const FileUtils::Fs &fs, const FileUtils::path &filepath,
              Renderer::Model *model, bool allow_reload = false);

  Errors::Result<Renderer::rTexture2D, int>
  ImportTexture(const FileUtils::Fs &fs, const FileUtils::path &filepath,
                bool allow_reload = false);

  Errors::Result<Renderer::rTexture2D, int>
  ImportTexture(const FileUtils::Fs &fs, const FileUtils::path &filepath,
                Renderer::Image *image, bool allow_reload = false);
};
