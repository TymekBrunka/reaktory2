#include <FileUtils.hpp>
#include <stdio.h>
namespace FileUtils {

bool ReadFilex(const std::filesystem::path &filepath, unsigned char *outbuffer,
               alloc_fun alloc, void *allocator) {

  FILE *file = fopen(filepath.c_str(), "rb");
  if (!file)
    return false;
}

} // namespace FileUtils
