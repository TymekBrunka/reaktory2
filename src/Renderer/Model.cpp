#include "assimp/mesh.h"
#include <AssimpGLMHelpers.hpp>
#include <Logging.hpp>
#include <Model.hpp>
#include <cstring>
#include <glad/gl.h>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

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

  model->processNode(scene->mRootNode, scene, initialise);

  model->initialised = initialise;
  return model;
}

void Model::processNode(void *node_, const void *scene_, bool initialise) {
  auto node = (aiNode *)node_;
  auto scene = (const aiScene *)scene_;
  std::cerr << "loading node named " << node->mName.data << "\n";
  // process all the node's meshes (if any)
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    int mesh_idx = node->mMeshes[i];
    std::cerr << "load mesh id#" << mesh_idx << "\n";
    meshes.push_back(
        processMesh(scene->mMeshes[node->mMeshes[i]], scene, initialise));
  }
  // then do the same for each of its children
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene, initialise);
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

} // namespace Renderer
