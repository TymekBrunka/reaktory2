#pragma once
#include <Errors/Errors.hpp>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
namespace FileUtils {

template <typename T, typename E> using Result = Errors::Result<T, E>;
using no_error = Errors::no_error;

extern std::filesystem::path HOME_DIR;
extern std::filesystem::path APP_ROOT;

typedef unsigned char *(*alloc_fun)(void *alloc, size_t n);
typedef unsigned char *(*free_fun)(void *alloc, void *buff, size_t n);

Result<unsigned char *, int> ReadFilex(const std::filesystem::path &filepath,
                                       alloc_fun alloc = nullptr,
                                       free_fun frre = nullptr,
                                       void *allocator = nullptr);

template <class Allocator = std::allocator<unsigned char>>
Result<unsigned char *, int>
ReadFile(const std::filesystem::path &filepath,
         const Allocator &alloc = std::allocator<unsigned char>()) {

  alloc_fun allo = [](void *aloc, size_t n) {
    return std::allocator_traits<Allocator>::allocate((Allocator &)aloc, n);
  };

  free_fun frre = [](void *aloc, void *buff, size_t n) {
    return std::allocator_traits<Allocator>::free((Allocator &)aloc, buff, n);
  };

  return ReadFilex(filepath, allo, frre, (void *)&alloc);
}

Result<no_error, int> WriteFile(const std::filesystem::path &filepath,
                                const void *data, size_t size);

Result<no_error, int>
WriteFileIfNotExists(const std::filesystem::path &filepath, const void *data,
                     size_t size);

} // namespace FileUtils
