#pragma once
#include <Errors/Errors.hpp>
#include <FileUtils.hpp>
#include <Model.hpp>
#include <Renderer.hpp>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

class ModelManager {
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
  ModelManager() = default;
  ModelManager(const std::filesystem::path &folder_);
  ~ModelManager() = default;
  ModelManager(const ModelManager &other) = delete;
  ModelManager &operator=(const ModelManager &other) = delete;
  ModelManager(ModelManager &&other) = default;
  ModelManager &operator=(ModelManager &&other) = default;

  inline const decltype(models)& GetModelsMap() const { return models; }

  inline Renderer::Model *operator[](const char *txt) {
    auto iter = models.find(txt);
    return iter != models.end() ? &(*iter).second : nullptr;
  }

  inline Renderer::Model *operator[](std::string_view txt) {
    auto iter = models.find(txt);
    return iter != models.end() ? &(*iter).second : nullptr;
  }

  inline Renderer::Model *operator[](const std::string &txt) {
    auto iter = models.find(txt);
    return iter != models.end() ? &(*iter).second : nullptr;
  }

  Errors::Result<Renderer::Model *, int>
  Import(const std::filesystem::path &filepath);

  Errors::Result<Renderer::Model *, int>
  Import(const std::filesystem::path &filepath, Renderer::Model *model);

  Errors::Result<Renderer::Model *, int> Load(const char *name);
};
