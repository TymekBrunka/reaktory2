#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
namespace Renderer {

struct modelVertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Material {
  unsigned int diffuse;
};

class Mesh {
};

} // namespace Renderer
