#pragma once
#include <Errors/Errors.hpp>
namespace Renderer {

template <typename T, typename E> using Result = Errors::Result<T, E>;
using Errors::no_error;

// vertex buffer object
typedef unsigned int rVBO;
// index buffer object
typedef unsigned int rEBO;
// vertex array object
typedef unsigned int rVAO;
// framebuffer object
typedef unsigned int rFBO;
// renderbuffer object
typedef unsigned int rRBO;
// uniform buffer object
typedef unsigned int rUBO;

typedef unsigned int rTextureAny;
typedef unsigned int rTexture2D;
typedef unsigned int rCubeMap;

typedef unsigned int rProgram;
typedef unsigned int rShader;

typedef int rLocation;

struct rect_size {
  int width;
  int height;
};

struct Image {
  int width;
  int height;
  int channels;
  int mipmap_levels;
  unsigned char *pixels;
};

class Camera;

class Render { // singleton since not all glfw callbacks provide user-data
                 // paramether
private:
  struct Impl;
  static Impl *impl;

public:
  static bool Init();
  static void Cleanup();
  static inline Impl *get_impl() { return impl; }

  static void SetWindowTitle(const char *title);
  static void SetWindowIcon(const Image &icon);
  static void SetCamera(const Camera *camera = nullptr);

  static Result<rProgram, no_error> LoadProgram(const char* vs, const char* fs);

  static Result<rTexture2D, no_error> LoadTexture(const Image& image);
  static void UnloadTexture(rTexture2D id);

  static rect_size GetRenderSize();
  static float GetDelta();

  static bool WindowShouldClose();
  static void BeginRenderPass(rFBO id, const rect_size viewport_size);
  static void EndRenderPass();
  static void BeginFrame();
  static void EndFrame();
};

} // namespace Renderer
