#include "Logging.hpp"
#include <Model.hpp>
#include <glad/gl.h>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
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
    const char* err = import.GetErrorString();
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Assimp", TL(MSG_ASSIMP_ERROR),
             std::make_format_args(err));
    return nullptr;
  }

  model->processNode(scene->mRootNode, scene);

  return model;
}

void Model::processNode(void *node, const void *scene) {

}

} // namespace Renderer
