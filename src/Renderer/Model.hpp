#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <string>
#include <string_view>
#include <unordered_map>
namespace Renderer {

// struct Material {
//   unsigned int diffuse1;
// };

struct meshVertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
  glm::vec3 Tangent;
  glm::vec3 Bitangent;

  int BoneIDs[8] = {0};
  float Weights[8] = {0};
};

// structure for temporary arrays to create opengl objects out of
struct MeshLoaderTmpCtx {
  struct Image {
    int width;
    int height;
    int channels;
    unsigned char *pixels;
  };

  std::vector<meshVertex> verticies;
  std::vector<unsigned int> indices;
  // std::vector<Material> materials;
  std::vector<Image> materials;
  // std::vector<aiNode *>
  std::vector<void *> nodes_refering_to_mesh;
  // std::vector<unsigned int> nodes_refering_to_mesh;
};

struct BoneInfo {
  int idx;
  glm::mat4x4 offset;
};

class MeshInstanced {
  friend class Model;
  MeshLoaderTmpCtx *ctx = nullptr;
  unsigned int VAO, InstanceVBO, VBO, EBO;

  MeshInstanced(const MeshLoaderTmpCtx &loader, bool initialise);

public:
  MeshInstanced() = default;
  ~MeshInstanced();
  MeshInstanced(const MeshInstanced &other) = delete;
  MeshInstanced &operator=(const MeshInstanced &other) = delete;
  MeshInstanced(MeshInstanced &&other);
  MeshInstanced &operator=(MeshInstanced &&other);
};

class Model {
  bool initialised;
  int boneCounter;

  int *mesh_idx_map = nullptr; // temporary array that maps aiNode.mMeeshes
                               // indexes to indexes in Model.meshes array
  std::vector<MeshInstanced> meshes;
  std::vector<unsigned int> owned_textures;

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

  std::unordered_map<std::string, BoneInfo, string_hash, std::equal_to<>>
      boneInfo;

  void processNode(void *node_, const void *scene_);
  MeshInstanced processMesh(void *mesh_, const void *scene_);

public:
  Model() = default;
  ~Model() = default;

  static Model *LoadFromFile(const char *filepath, bool initialise);
  void render();
};

} // namespace Renderer
