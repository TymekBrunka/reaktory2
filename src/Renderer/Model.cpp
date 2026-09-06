#include "assimp/anim.h"
#include "assimp/mesh.h"
#include <AssimpGLMHelpers.hpp>
#include <Logging.hpp>
#include <Model.hpp>
#include <cstring>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cmath>
#include <iostream>
namespace Renderer {

#define MAX_BONES 100

Mesh::~Mesh() {
  if (ctx)
    delete[] ctx;
  if (VAO)
    glDeleteVertexArrays(1, &VAO);
  if (VBO)
    glDeleteBuffers(1, &VBO);
  if (EBO)
    glDeleteBuffers(1, &EBO);
}

Mesh::Mesh(Mesh &&other) {
  ctx = other.ctx;
  VAO = other.VAO;
  VBO = other.VBO;
  EBO = other.EBO;
  numIndices = other.numIndices;
  ctx = other.ctx;
  other.VAO = 0;
  other.VBO = 0;
  other.EBO = 0;
  other.ctx = nullptr;
}

Mesh &Mesh::operator=(Mesh &&other) {
  if (this != &other) {
    ctx = other.ctx;
    VAO = other.VAO;
    VBO = other.VBO;
    EBO = other.EBO;
    ctx = other.ctx;
    numIndices = other.numIndices;
    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;
    other.ctx = nullptr;
  }
  return *this;
}

void Mesh::init() {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(GL_ARRAY_BUFFER, ctx->vertices.size() * sizeof(meshVertex),
               &ctx->vertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               ctx->indices.size() * sizeof(unsigned int), &ctx->indices[0],
               GL_STATIC_DRAW);

  // vertex positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(meshVertex),
                        (void *)0);
  // vertex normals
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(meshVertex),
                        (void *)offsetof(meshVertex, Normal));
  // vertex texture coords
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(meshVertex),
                        (void *)offsetof(meshVertex, TexCoords));
  // ids
  glEnableVertexAttribArray(3);
  glVertexAttribIPointer(3, 4, GL_INT, sizeof(meshVertex),
                         (void *)(0 + offsetof(meshVertex, BoneIdxs)));

  glEnableVertexAttribArray(4);
  glVertexAttribIPointer(
      4, 4, GL_INT, sizeof(meshVertex),
      (void *)((4 * sizeof(int)) + offsetof(meshVertex, BoneIdxs)));

  // weights
  glEnableVertexAttribArray(5);
  glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(meshVertex),
                        (void *)(0 + offsetof(meshVertex, Weights)));

  glEnableVertexAttribArray(6);
  glVertexAttribPointer(
      6, 4, GL_FLOAT, GL_FALSE, sizeof(meshVertex),
      (void *)(4 * sizeof(float) + offsetof(meshVertex, Weights)));

  glBindVertexArray(0);

  numIndices = ctx->indices.size();
  delete ctx;
  ctx = nullptr;
}

void Mesh::Draw() {
  glBindVertexArray(VAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Model::Draw() {
  for (auto &mesh : meshes)
    mesh.Draw();
}

void Model::PrintNodeTree() const {
  std::cerr << "Model node tree:\n";
  PrintNodeTreeImpl(&nodes[0], 0);
};

void Model::PrintNodeTreeImpl(const modelNode *node, int depth) const {
  for (int i = 0; i < depth; i++)
    std::cerr << "  ";
  std::cerr << "(" << node->idx << ") " << node->name << "\n";
  for (int i = 0; i < node->childCount; i++) {
    PrintNodeTreeImpl(&nodes[node->childStartIdx + i], depth + 1);
  }
}

Model *Model::LoadFromFile(const char *filepath, bool initialise) {
  Model *model = new Model{};

  Assimp::Importer import;
  const aiScene *scene =
      import.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    const char *err = import.GetErrorString();
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Assimp", TL(MSG_ASSIMP_ERROR),
             std::make_format_args(err));
    return nullptr;
  }

  // this would go to animation class but im only loading animations from
  // within
  // single model, so its best to save space per animation and save node
  // hiererchy here

  model->nodes.push_back(modelNode{.idx = 0});
  model->processNode(scene->mRootNode, scene, 0, initialise);

  model->PrintNodeTree();

  for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
    model->loadAnimation(scene->mAnimations[i]);
  }

  for (int i = 0; i < MAX_BONES; i++)
    model->finalMatrices[i] = glm::mat4(1.0f);

  model->initialised = initialise;
  return model;
}

void Model::processNode(void *node_, const void *scene_, int nodeIdx,
                        bool initialise) {
  auto node = (aiNode *)node_;
  auto scene = (const aiScene *)scene_;

  modelNode &self = nodes[nodeIdx];
  self.childCount = node->mNumChildren;
  self.childStartIdx = (int)nodes.size();
  self.transformation =
      AssimpGLMHelpers::ConvertMatrixToGLMFormat(node->mTransformation);
  self.name = node->mName.data;
  std::cerr << "loading node named " << self.name << " that has "
            << self.childCount << " child nodes\n";

  int childStartIdx = self.childStartIdx; // make copy before it's too late
  // fill child node 'array' before recuring (so they stay next to each other)
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    nodes.push_back(modelNode{.idx = (int)nodes.size()});
  }

  // process all the node's meshes (if any)
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    int mesh_idx = node->mMeshes[i];
    std::cerr << "load mesh id#" << mesh_idx << "\n";
    meshes.push_back(processMesh(scene->mMeshes[mesh_idx], scene, initialise));
  }

  // process each of aiNode's children
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene, childStartIdx + i, initialise);
  }
}

Mesh Model::processMesh(void *mesh_, const void *scene_, bool initialise) {
  auto mesh = (aiMesh *)mesh_;
  auto scene = (const aiScene *)scene_;

  std::cerr << "loading mesh named " << mesh->mName.data << "\n";

  Mesh Mesh{};
  Mesh.ctx = new MeshLoaderTmpCtx{};

  std::cerr << "mesh consists of " << mesh->mNumVertices << " vertices\n";
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    meshVertex vertex;
    vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
    vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);
    // i only support 1 texcoord per vertex
    if (mesh->mTextureCoords[0]) {
      glm::vec2 vec;
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;
      vertex.TexCoords = vec;
    } else
      vertex.TexCoords = glm::vec2(0.0f, 0.0f);

    Mesh.ctx->vertices.push_back(vertex);
  }

  std::cerr << "mesh consists of ~" << mesh->mNumFaces * 3 << " indices\n";
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      Mesh.ctx->indices.push_back(face.mIndices[j]);
  }

  ExtractBoneWeightForVertices(Mesh.ctx->vertices, mesh_, scene_);

  std::cerr << "mesh bone:index map:\n";
  for (const auto &[name, data] : boneInfoMap) {
    std::cerr << "  " << data.idx << " - " << name << "\n";
  }

  if (initialise)
    Mesh.init();

  return Mesh;
}

static void SetVertexBoneData(meshVertex &vertex, int boneIdx, float weight) {
  for (int i = 0; i < 8; i++) {
    if (vertex.BoneIdxs[i] < 0) {
      vertex.Weights[i] = weight;
      vertex.BoneIdxs[i] = boneIdx;
      break;
    }
  }
}

void Model::ExtractBoneWeightForVertices(std::vector<meshVertex> &vertices,
                                         void *mesh_, const void *scene_) {
  auto mesh = (aiMesh *)mesh_;
  auto scene = (const aiScene *)scene_;

  for (int i = 0; i < mesh->mNumBones; i++) {
    int boneIdx = -1;
    const char *boneName = mesh->mBones[i]->mName.data;

    if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
      boneInfoMap[boneName] =
          BoneInfo{.idx = boneCounter,
                   .offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(
                       mesh->mBones[i]->mOffsetMatrix)};
      boneIdx = boneCounter++;
      std::cerr << "Loaded mesh bone " << boneName << "\n";
    } else
      boneIdx = (*boneInfoMap.find(boneName)).second.idx;

    aiVertexWeight *weights = mesh->mBones[i]->mWeights;
    int numWeights = mesh->mBones[i]->mNumWeights;

    for (int w = 0; w < numWeights; w++) {
      if (weights[w].mVertexId >= vertices.size())
        continue;

      SetVertexBoneData(vertices[w], boneIdx, weights[w].mWeight);
    }
  }
}

AnimationBoneChannel::AnimationBoneChannel(const std::string &name,
                                           void *channel_) {
  auto channel = (aiNodeAnim *)channel_;
  this->name = name;
  std::cerr << "loading animation channel named " << name << "\n";

  for (int i = 0; i < channel->mNumPositionKeys; ++i) {
    Positions.push_back(KeyPosition{
        .position =
            AssimpGLMHelpers::GetGLMVec(channel->mPositionKeys[i].mValue),
        .timeStamp = (float)channel->mPositionKeys[i].mTime,
    });
  }

  for (int i = 0; i < channel->mNumRotationKeys; ++i) {
    Rotations.push_back(KeyRotation{
        .orientation =
            AssimpGLMHelpers::GetGLMQuat(channel->mRotationKeys[i].mValue),
        .timeStamp = (float)channel->mRotationKeys[i].mTime,
    });
  }

  for (int i = 0; i < channel->mNumScalingKeys; ++i) {
    Scales.push_back(KeyScale{
        .scale = AssimpGLMHelpers::GetGLMVec(channel->mScalingKeys[i].mValue),
        .timeStamp = (float)channel->mScalingKeys[i].mTime,
    });
  }
}

int AnimationBoneChannel::GetPositionIndex(float animationTime) {
  for (int index = 0; index < Positions.size() - 1; ++index) {
    if (animationTime < Positions[index + 1].timeStamp)
      return index;
  }
  // assert(0);
  return 0;
}

int AnimationBoneChannel::GetRotationIndex(float animationTime) {
  for (int index = 0; index < Rotations.size() - 1; ++index) {
    if (animationTime < Rotations[index + 1].timeStamp)
      return index;
  }
  // assert(0);
  return 0;
}

int AnimationBoneChannel::GetScaleIndex(float animationTime) {
  for (int index = 0; index < Scales.size() - 1; ++index) {
    if (animationTime < Scales[index + 1].timeStamp)
      return index;
  }
  // assert(0);
  return 0;
}

float AnimationBoneChannel::GetScaleFactor(float lastTimeStamp,
                                           float nextTimeStamp,
                                           float animationTime) {
  float midWayLength = animationTime - lastTimeStamp;
  float framesDiff = nextTimeStamp - lastTimeStamp;
  float scaleFactor = midWayLength / framesDiff;
  return scaleFactor;
}

void AnimationBoneChannel::UpdateTo(float animationTime) {
  glm::mat4 translation{};
  if (1 == Positions.size())
    translation = glm::translate(glm::mat4(1.0f), Positions[0].position);
  else {
    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor =
        GetScaleFactor(Positions[p0Index].timeStamp,
                       Positions[p1Index].timeStamp, animationTime);
    glm::vec3 finalPosition = glm::mix(
        Positions[p0Index].position, Positions[p1Index].position, scaleFactor);
    translation = glm::translate(glm::mat4(1.0f), finalPosition);
  }

  glm::mat4 rotation{};
  if (1 == Rotations.size()) {
    rotation = glm::mat4(glm::normalize(Rotations[0].orientation));
  } else {
    int p0Index = GetRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor =
        GetScaleFactor(Rotations[p0Index].timeStamp,
                       Rotations[p1Index].timeStamp, animationTime);
    glm::quat finalRotation =
        glm::slerp(Rotations[p0Index].orientation,
                   Rotations[p1Index].orientation, scaleFactor);
    finalRotation = glm::normalize(finalRotation);
    rotation = glm::mat4(finalRotation);
  }

  glm::mat4 scale{};
  if (1 == Scales.size())
    scale = glm::scale(glm::mat4(1.0f), Scales[0].scale);
  else {
    int p0Index = GetScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(
        Scales[p0Index].timeStamp, Scales[p1Index].timeStamp, animationTime);
    glm::vec3 finalScale =
        glm::mix(Scales[p0Index].scale, Scales[p1Index].scale, scaleFactor);
    scale = glm::scale(glm::mat4(1.0f), finalScale);
  }

  localTransform = translation * rotation * scale;
}

void Model::loadAnimation(void *animation_) {
  auto animation = (aiAnimation *)animation_;
  std::cerr << "loading animation named " << animation->mName.data << "\n";

  Animation animaton;
  animaton.name = animation->mName.data;
  animaton.TicksPerSecond = animation->mTicksPerSecond;
  for (int i = 0; i < animation->mNumChannels; i++) {
    animaton.channels.push_back(std::move(AnimationBoneChannel(
        animation->mChannels[i]->mNodeName.data, animation->mChannels[i])));
  }

  animations.push_back(std::move(animaton));
}

void Model::SetAnimation(const Animation *animation) {
  current_animation = (Animation *)animation;

  if (animation) {
    for (auto &node : nodes) {
      int channel_idx = -1;
      BoneInfo *bone_binding = nullptr;
      int i = 0;
      for (const auto &channel : animation->channels) {
        if (channel.name == node.name) {
          channel_idx = i;
        }
        i++;
      }
      auto fnd = boneInfoMap.find(node.name);
      if (fnd != boneInfoMap.end())
        bone_binding = &(*fnd).second;

      node.channel_binding = channel_idx;
      node.bone_idx_binding = bone_binding;
    }
  } else {
    for (auto &node : nodes) {
      node.channel_binding = -1;
      node.bone_idx_binding = nullptr;
    }
  }

  animationTime = 0;
}

void Model::Advance(float delta) {
  if (current_animation) {
    animationTime += current_animation->TicksPerSecond * delta;
    animationTime = fmod(animationTime, current_animation->Duration);
    calculateBoneTransform(&nodes[0], glm::mat4(1.0));
  }
}

void Model::calculateBoneTransform(modelNode *node, glm::mat4 parentTransform) {
  glm::mat4 nodeTransform = node->transformation;

  if (node->channel_binding != -1) {
    auto &channel = current_animation->channels[node->channel_binding];
    channel.UpdateTo(animationTime);
    nodeTransform = channel.localTransform;
  }

  glm::mat4 globalTransform = parentTransform * nodeTransform;

  if (node->bone_idx_binding) {
    finalMatrices[node->bone_idx_binding->idx] =
        globalTransform * node->bone_idx_binding->offset;
  }

  for (int i = 0; i < node->childCount; i++) {
    calculateBoneTransform(&nodes[node->childStartIdx + i], globalTransform);
  }
}

} // namespace Renderer
