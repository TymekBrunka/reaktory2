#pragma once
#include "FileUtils.hpp"
#include "Renderer.hpp"
#include <cstddef>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <string>
#include <string_view>
#include <unordered_map>
namespace Renderer {

template <typename T, typename E> using Result = Errors::Result<T, E>;
using no_error = Errors::no_error;

struct Material {
  unsigned int diffuse1 = -1;
  unsigned int program = -1;
  float color_diffuse[4] = {1, 1, 1, 1};
};

struct meshVertex {
  glm::vec3 Position{};
  glm::vec3 Normal{};
  glm::vec2 TexCoords{};
  glm::vec3 Tangent{};
  glm::vec3 Bitangent{};

  int BoneIdxs[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
  float Weights[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
};

// structure for temporary arrays to create opengl objects out of
struct MeshLoaderTmpCtx {
  struct MaterialTmpCtx {
    glm::vec4 color_diffuse{};
    std::string diffuse1;
  };

  std::vector<meshVertex> vertices{};
  std::vector<unsigned int> indices{};
  MaterialTmpCtx material;
};

struct BoneInfo {
  int idx = 0;
  glm::mat4 offset{1.0f};
};

class Mesh {
  friend class Model;
  static unsigned int defaultProgram;
  unsigned int numIndices = 0;
  unsigned int VAO = 0, VBO = 0, EBO = 0;
  // Material material;
  MeshLoaderTmpCtx *ctx = nullptr;
  std::string name;

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

struct KeyPosition {
  glm::vec3 position{};
  float timeStamp = 0.0f;
};

struct KeyRotation {
  glm::quat orientation{};
  float timeStamp = 0.0f;
};

struct KeyScale {
  glm::vec3 scale{};
  float timeStamp = 0.0f;
};

class AnimationBoneChannel {
  friend class Model;

private:
  std::vector<KeyPosition> Positions{};
  std::vector<KeyRotation> Rotations{};
  std::vector<KeyScale> Scales{};

  glm::mat4 localTransform{1.0f};
  std::string name{};

  AnimationBoneChannel(const std::string &name, void *channel_);

  void UpdateTo(float animationTime);
  int GetPositionIndex(float animationTime);
  int GetRotationIndex(float animationTime);
  int GetScaleIndex(float animationTime);

  /* Gets normalized value for Lerp & Slerp*/
  float GetScaleFactor(float lastTimeStamp, float nextTimeStamp,
                       float animationTime);

public:
  ~AnimationBoneChannel() = default;
  // AnimationBoneChannel(const AnimationBoneChannel &other) = default;
  // AnimationBoneChannel &operator=(const AnimationBoneChannel &other) =
  // default;
  AnimationBoneChannel(AnimationBoneChannel &&other);
  AnimationBoneChannel &operator=(AnimationBoneChannel &&other);
};

struct Animation {
  friend class Model;

private:
  float Duration = 0.0f;
  int TicksPerSecond = 0;
  std::string name{};
  std::vector<AnimationBoneChannel> channels{};

public:
  inline float GetDuration() const { return Duration; }
  inline int GetTPS() const { return TicksPerSecond; }
  inline const std::string &GetName() const { return name; }
};

// matrices calculated for animations are calculated from root node to leafs,
// which requires storing node hierarchy
struct modelNode {
  int idx = 0;
  int channel_binding = -1;
  BoneInfo *bone_idx_binding = nullptr;
  int childCount = 0;
  int childStartIdx =
      -1; // those nodes will be stored in continous array but when loading, it
          // will be changing its size which would invalidate pointers, so index
          // has to be used instead
  glm::mat4 transformation{};
  std::string name{};
};

class Model {
  bool initialised = false;
  float animationTime = 0;
  int boneCounter = 0;

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

  struct Image {
    int width = 0;
    int height = 0;
    int channels = 4;
    unsigned char *pixels = nullptr;
  };

  std::unordered_map<std::string, Image, string_hash, std::equal_to<>>
      *texture_data = nullptr;

  Animation *current_animation =
      nullptr; // reset to null upon std::move() bc i won't be moving it while
               // it's animated anytime soon

  std::vector<modelNode>
      nodes; // normally, those 2 fields should be stored in animation class but
             // since im not loading animations outside the model, i can safetly
             // put it here and optimise some other things

  std::vector<Mesh> meshes{};
  std::vector<Animation> animations{};
  std::vector<Material> materials{};

  std::unordered_map<std::string, BoneInfo, string_hash, std::equal_to<>>
      boneInfoMap{};

  glm::mat4 finalMatrices[100]{};

  void PrintNodeTreeImpl(const modelNode *node, int depth) const;

  void processNode(void *node_, const void *scene_, int nodeIdx,
                   bool initialise);
  Mesh processMesh(void *mesh_, const void *scene_, bool initialise);
  void ExtractBoneWeightForVertices(std::vector<meshVertex> &vertices,
                                    void *mesh_, const void *scene_);

  void loadAnimation(void *animation_);

  void calculateBoneTransform(modelNode *node, glm::mat4 parentTransform);

  static bool LoadModel(const FileUtils::Fs &fs, const void *scene_,
                        Model &model, bool initialise = true);

public:
  Model() = default;
  ~Model();
  Model(const Model &other) = delete;
  Model &operator=(const Model &other) = delete;
  Model(Model &&other);
  Model &operator=(Model &&other);

  static Model LoadFromFile(const FileUtils::Fs &fs,
                            const FileUtils::path &filepath,
                            bool initialise = true);
  // static Model LoadFromMemory(const FileUtils::Fs &fs, const void *data,
  //                             size_t length, bool initialise = true,
  //                             const char *hint = "");

  void init();

  inline const std::vector<Animation> &GetAnimations() const {
    return animations;
  }

  inline float &GetAnimationTime() { return animationTime; }

  inline const glm::mat4 *GetFinalMatrices() const { return finalMatrices; }
  inline std::vector<Material> &GetMaterials() { return materials; }

  inline int FindMesh(const char *name) {
    for (int i = 0; i < meshes.size(); i++) {
      if (meshes[i].name == name) {
        return i;
      }
    }
    return -1;
  }

  void SetAnimation(const Animation *animation);
  void Advance(float delta);

  void PrintNodeTree() const;
  void Draw();
};

} // namespace Renderer
