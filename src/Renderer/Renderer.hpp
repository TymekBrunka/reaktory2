#pragma once
#define GLFW_INCLUDE_NONE
#include <Errors/Errors.hpp>
#include <GLFW/glfw3.h>
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

typedef float vec2[2];
typedef float vec3[3];
typedef float vec4[4];
typedef float mat2[4];
typedef float mat3[9];
typedef float mat4[16];

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

// class Camera;

class Render { // singleton since not all glfw callbacks provide user-data
               // paramether
private:
  struct Impl;
  static void *window_1st;
  static double timeout;
  static bool initialised;

public:
  typedef void (*RENDERframebuffersizefun)(Render &window, int width,
                                           int height);
  typedef void (*RENDERwindowminimisefun)(Render &window, int iconified);
  typedef void (*RENDERwindowmaximizefun)(Render &window, int maximized);

  typedef void (*RENDERkeyfun)(Render &window, int key, int scancode,
                               int action, int mods);
  typedef void (*RENDERcursorposfun)(Render &window, double xpos, double ypos);
  typedef void (*RENDERmousebuttonfun)(Render &window, int button, int action,
                                       int mods);

  typedef void (*RENDERscrollfun)(Render &window, double xoffset,
                                  double yoffset);
  typedef void (*RENDERdropfun)(Render &window, int path_count,
                                const char *paths[]);

  void *userdata = nullptr;
  Impl *impl = nullptr;

  static bool Init();
  static void Cleanup();
  static void PullEvents();
  static void SetTargetFPS(double fps);
  void makeContextCurrent();

  Render() = default;
  ~Render();
  bool init(const char *title, rect_size window_size);
  void cleanup();
  Render(const Render &other) = delete;
  Render &operator=(const Render &other) = delete;
  Render(Render &&other);
  Render &operator=(Render &&other);

  void SetWindowTitle(const char *title);
  void SetWindowIcon(const Image &icon_);
  // void SetCamera(const Camera *camera = nullptr);

  template <typename T> T &GetUserData() { return *(T *)userdata; }
  GLFWwindow *GetGLFWWindow();
  bool IsMinimised() const;
  void SetResizeCallback(RENDERframebuffersizefun callback);
  void SetMinimiseCallback(RENDERwindowminimisefun callback);
  void SetMaximiseCallback(RENDERwindowmaximizefun callback);
  void SetKeyCallback(RENDERkeyfun callback);
  void SetMouseMoveCallback(RENDERcursorposfun callback);
  void SetMouseButtonCallback(RENDERmousebuttonfun callback);
  void SetScrollCallback(RENDERscrollfun callback);
  void SetDropCallback(RENDERdropfun callback);

  rect_size GetCursorPosition() const;

  Result<rProgram, no_error> LoadProgram(const char *name, const char *vs,
                                         const char *fs);
  void UnloadProgram(rProgram program);

  Result<Image, no_error> LoadImageFromMemory(const unsigned char *data,
                                              int length,
                                              int desired_channels = 0);
  Result<rTexture2D, no_error>
  LoadTexture(const Image &image, bool pixelated = false, bool repeat = false);
  void BindTexture(rTexture2D id, int slot);
  void UnloadTexture(rTexture2D id);

  rect_size GetRenderSize() const;
  float GetDelta() const;

  bool WindowShouldClose() const;
  void BeginRenderPass(rFBO id, const rect_size viewport_size);
  void EndRenderPass();
  void BeginFrame();
  void EndFrame();
  void ClearBackground();
  void ClearColor();
  void ClearDepth();
};

} // namespace Renderer
