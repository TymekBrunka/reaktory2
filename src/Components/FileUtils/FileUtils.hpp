#pragma once
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
namespace FileUtils {

std::filesystem::path HOME_DIR;
std::filesystem::path APP_ROOT;

typedef unsigned char *(*alloc_fun)(void *alloc, size_t n);

bool ReadFilex(const std::filesystem::path &filepath, unsigned char *outbuffer,
               alloc_fun alloc = nullptr, void *allocator = nullptr);

template <class Allocator = std::allocator<unsigned char>>
bool ReadFile(const std::filesystem::path &filepath, unsigned char *outbuffer,
                        const Allocator &alloc) {

  alloc_fun allo = [](void *aloc, size_t n) {
    return std::allocator_traits<Allocator>::allocate((Allocator &)aloc, n);
  };

  return ReadFilex(filepath, outbuffer, allo, (void *)&alloc);
}

} // namespace FileUtils
