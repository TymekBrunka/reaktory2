#pragma once
#include <Errors/Errors.hpp>
#include <FileUtils.hpp>
#include <Model.hpp>
#include <Renderer.hpp>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

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

private:
  std::filesystem::path folder{};

  std::unordered_map<std::string, Renderer::Model, string_hash, std::equal_to<>>
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

  inline Renderer::Model *GetModel(const char *txt) {
    auto iter = models.find(txt);
    return iter != models.end() ? &(*iter).second : nullptr;
  }

  inline Renderer::Model *GetModel(std::string_view txt) {
    auto iter = models.find(txt);
    return iter != models.end() ? &(*iter).second : nullptr;
  }

  inline Renderer::Model *GetModel(const std::string &txt) {
    auto iter = models.find(txt);
    return iter != models.end() ? &(*iter).second : nullptr;
  }

  Errors::Result<Renderer::Model *, int>
  ImportModel(const FileUtils::Fs &fs, const std::filesystem::path &filepath,
              bool allow_reload = false);

  Errors::Result<Renderer::Model *, int>
  ImportModel(const FileUtils::Fs &fs, const std::filesystem::path &filepath,
              Renderer::Model *model, bool allow_reload = false);

  Errors::Result<Renderer::Model *, int> LoadModel(const char *name);
};
