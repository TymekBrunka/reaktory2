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
private:
  std::filesystem::path folder{};

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

  std::unordered_map<std::string, Renderer::Model, string_hash, std::equal_to<>>
      models;

public:
  ResourceManager() = default;
  ResourceManager(const std::filesystem::path &folder_) : folder(folder_) {};
  ~ResourceManager() = default;
  ResourceManager(const ResourceManager &other) = delete;
  ResourceManager &operator=(const ResourceManager &other) = delete;
  ResourceManager(ResourceManager &&other) = default;
  ResourceManager &operator=(ResourceManager &&other) = default;

  inline const decltype(models)& GetModelsMap() const { return models; }

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
  ImportModel(const std::filesystem::path &filepath, bool allow_reload = false);

  Errors::Result<Renderer::Model *, int>
  ImportModel(const std::filesystem::path &filepath, Renderer::Model *model, bool allow_reload = false);

  Errors::Result<Renderer::Model *, int> LoadModel(const char *name);
};
