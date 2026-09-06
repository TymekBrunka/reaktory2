#pragma once
#include <glm/gtc/quaternion.hpp>
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
  struct Image {
    int width = 0;
    int height = 0;
    int channels = 4;
    unsigned char *pixels = nullptr;
  };

  Image material{};
  std::vector<meshVertex> vertices{};
  std::vector<unsigned int> indices{};
};

struct BoneInfo {
  int idx = 0;
  glm::mat4x4 offset{};
};

class Mesh {
  friend class Model;
  unsigned int numIndices = 0;
  unsigned int VAO = 0, VBO = 0, EBO = 0;
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

  glm::mat4 localTransform{};
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
  Animation *current_animation = nullptr;

  std::vector<modelNode>
      nodes; // normally, those 2 fields should be stored in animation class but
             // since im not loading animations outside the model, i can safetly
             // put it here and optimise some other things

  std::vector<Mesh> meshes{};
  std::vector<Animation> animations{};
  std::vector<unsigned int> owned_textures{};
  glm::mat4 finalMatrices[100]{};

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
      boneInfoMap{};

  void PrintNodeTreeImpl(const modelNode *node, int depth) const;

  void processNode(void *node_, const void *scene_, int nodeIdx,
                   bool initialise);
  Mesh processMesh(void *mesh_, const void *scene_, bool initialise);
  void ExtractBoneWeightForVertices(std::vector<meshVertex> &vertices,
                                    void *mesh_, const void *scene_);

  void loadAnimation(void *animation_);

  void calculateBoneTransform(modelNode *node, glm::mat4 parentTransform);

public:
  Model() = default;
  ~Model() = default;

  static Model *LoadFromFile(const char *filepath, bool initialise = true);
  inline const std::vector<Animation> &GetAnimations() const {
    return animations;
  }

  inline const glm::mat4 *GetFinalMatrices() const { return finalMatrices; }

  void SetAnimation(const Animation *animation);
  void Advance(float delta);

  void PrintNodeTree() const;
  void Draw();
};

} // namespace Renderer
