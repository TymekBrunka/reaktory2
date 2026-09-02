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

MeshInstanced::~MeshInstanced() {
  if (ctx)
    delete[] ctx;
  if (VAO)
    glDeleteVertexArrays(1, &VAO);
  if (VBO)
    glDeleteBuffers(1, &VBO);
  if (EBO)
    glDeleteBuffers(1, &EBO);
}

MeshInstanced::MeshInstanced(MeshInstanced &&other) {
  ctx = other.ctx;
  VAO = other.VAO;
  VBO = other.VBO;
  EBO = other.EBO;
  ctx = other.ctx;
  other.VAO = 0;
  other.VBO = 0;
  other.EBO = 0;
  other.ctx = nullptr;
}

MeshInstanced &MeshInstanced::operator=(MeshInstanced &&other) {
  if (this != &other) {
    ctx = other.ctx;
    VAO = other.VAO;
    VBO = other.VBO;
    EBO = other.EBO;
    ctx = other.ctx;
    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;
    other.ctx = nullptr;
  }
  return *this;
}

Model *Model::LoadFromFile(const char *filepath, bool initialise) {
  Model *model = new Model();

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

  model->mesh_idx_map = new int[scene->mNumMeshes];
  for (int i = 0; i < scene->mNumMeshes; i++) {
    model->mesh_idx_map[i] = -1;
  }
  model->processNode(scene->mRootNode, scene);

  delete[] model->mesh_idx_map;

  return model;
}

void Model::processNode(void *node_, const void *scene_) {
  auto node = (aiNode *)node_;
  auto scene = (const aiScene *)scene_;
  std::cerr << "loading node named " << node->mName.data << "\n";
  // process all the node's meshes (if any)
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    int mesh_idx = node->mMeshes[i];
    std::cerr << "load mesh id#" << mesh_idx << "\n";
    if (mesh_idx_map[mesh_idx] == -1)
      meshes.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene));
    else
      meshes[mesh_idx_map[mesh_idx]].ctx->nodes_refering_to_mesh.push_back(
          node);
  }
  // then do the same for each of its children
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene);
  }
}

MeshInstanced Model::processMesh(void *mesh_, const void *scene_) {
  auto mesh = (aiMesh *)mesh_;
  auto scene = (const aiScene *)scene_;

  std::cerr << "loading mesh named " << mesh->mName.data << "\n";

  MeshInstanced Mesh{};
  Mesh.ctx = new MeshLoaderTmpCtx{};

  std::cerr << "mesh consists of " << mesh->mNumVertices << " verticies\n";
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    meshVertex vertex;
    vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
    vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);
    if (mesh->mTextureCoords[0]) {
      glm::vec2 vec;
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;
      vertex.TexCoords = vec;
    } else
      vertex.TexCoords = glm::vec2(0.0f, 0.0f);

    Mesh.ctx->verticies.push_back(vertex);
  }

  std::cerr << "mesh consists of ~" << mesh->mNumFaces * 3 << " indices\n";
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      Mesh.ctx->indices.push_back(face.mIndices[j]);
  }

  return Mesh;
}

} // namespace Renderer
