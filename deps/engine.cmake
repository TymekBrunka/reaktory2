#math
message(glm)
CPMAddPackage(
  NAME glm
  VERSION 1.0.3
  GITHUB_REPOSITORY g-truc/glm
  GIT_TAG 1.0.3
  OPTIONS
    "GLM_ENABLE_CXX_20 ON"
)

# rendering
message(glfw)
CPMAddPackage(
  NAME glfw3
  VERSION 3.4
  GITHUB_REPOSITORY glfw/glfw
  GIT_TAG 3.4
  OPTIONS
    "GLFW_BUILD_EXAMPLES OFF"
    "GLFW_BUILD_TESTS OFF"
    "GLFW_BUILD_DOCS OFF"
)

message(glad)
add_library(glad OBJECT deps/glad/src/gl.c)
target_include_directories(glad PUBLIC deps/glad/include/)

file(GLOB imgui_SRC
  deps/imgui/*.hpp
  deps/imgui/*.cpp
)

message(imgui)
add_library(imgui OBJECT ${imgui_SRC})
target_include_directories(imgui PUBLIC deps/imgui)
target_compile_options(imgui PRIVATE "-DIMGUI_IMPL_OPENGL_LOADER_CUSTOM <glad/gl.h>")
target_link_libraries(imgui glad glfw)

message(imguizmo)
CPMAddPackage(
  NAME imguizmo
  VERSION 1.90
  GITHUB_REPOSITORY cedricguillemet/imguizmo
  GIT_TAG 1.9
)

target_compile_features(imguizmo PRIVATE cxx_std_11)
target_link_libraries(imguizmo PUBLIC imgui)

#loading models
message(assimp)
CPMAddPackage(
  NAME assimp
  VERSION 6.0.4
  GITHUB_REPOSITORY assimp/assimp
  GIT_TAG v6.0.4
  OPTIONS
    "BUILD_SHARED_LIBS OFF"
    "ASSIMP_BUILD_TESTS OFF"
    "ASSIMP_INSTALL OFF"
    "ASSIMP_BUILD_DOCS OFF"
    "ASSIMP_BUILD_ZLIB OFF"
    "ASSIMP_WARNINGS_AS_ERRORS OFF" #set but unused error causes assimp not to build

    "ASSIMP_BUILD_M3D_IMPORTER ON"
    # "ZLIB_LIBRARY ${zlib_LIBRARIES}"
    # "ZLIB_INCLUDE_DIR ${zlib_INCLUDE_DIRS}"
)
target_link_libraries(assimp libminizipstatic) # assimp forgets to link with minizip in this case
