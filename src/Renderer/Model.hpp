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

  int BoneIdxs[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
  float Weights[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
};

// structure for temporary arrays to create opengl objects out of
struct MeshLoaderTmpCtx {
  struct Image {
    int width;
    int height;
    int channels;
    unsigned char *pixels;
  };

  Image material;
  std::vector<meshVertex> vertices;
  std::vector<unsigned int> indices;
};

struct BoneInfo {
  int idx;
  glm::mat4x4 offset;
};

class Mesh {
  friend class Model;
  unsigned int numIndices;
  unsigned int VAO, VBO, EBO;
  MeshLoaderTmpCtx *ctx = nullptr;

  void init();
  void Draw();

public:
  Mesh() = default;
  ~Mesh();
  Mesh(const Mesh &other) = delete;
  Mesh &operator=(const Mesh &other) = delete;
  Mesh(Mesh &&other);
  Mesh &operator=(Mesh &&other);
};

class Model {
  bool initialised;
  int boneCounter;

  std::vector<Mesh> meshes;
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
      boneInfoMap;

  void processNode(void *node_, const void *scene_, bool initialise);
  Mesh processMesh(void *mesh_, const void *scene_, bool initialise);
  void ExtractBoneWeightForVertices(std::vector<meshVertex> &vertices, void *mesh_, const void *scene_);

public:
  Model() = default;
  ~Model() = default;

  static Model *LoadFromFile(const char *filepath, bool initialise = true);
  void Draw();
};

} // namespace Renderer
