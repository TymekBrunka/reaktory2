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
  Impl *impl = nullptr;

public:
  static bool Init();
  static void Cleanup();
  void makeContextCurrent();
  inline Impl *get_impl() { return impl; }

  Render() = default;
  ~Render();
  Render(const char *title, rect_size window_size);
  Render(const Render &other) = delete;
  Render &operator=(const Render &other) = delete;
  Render(Render &&other);
  Render &operator=(Render &&other);

  void SetWindowTitle(const char *title);
  void SetWindowIcon(const Image &icon_);
  void SetCamera(const Camera *camera = nullptr);

  Result<rProgram, no_error> LoadProgram(const char *name, const char *vs,
                                         const char *fs);
  void UnloadProgram(rProgram program);

  Result<rTexture2D, no_error> LoadTexture(const Image &image);
  void UnloadTexture(rTexture2D id);

  rect_size GetRenderSize();
  float GetDelta();

  bool WindowShouldClose();
  void BeginRenderPass(rFBO id, const rect_size viewport_size);
  void EndRenderPass();
  void BeginFrame();
  void EndFrame();
  void ClearBackground();
  void ClearColor();
  void ClearDepth();
};

} // namespace Renderer
