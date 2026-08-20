#include <FileUtils.hpp>
#include <cstddef>
#include <stdio.h>
namespace FileUtils {

std::filesystem::path HOME_DIR = "";
std::filesystem::path APP_ROOT = "";

Result<unsigned char *, int> ReadFilex(const std::filesystem::path &filepath,
                                       alloc_fun alloc, void *allocator) {

  FILE *file = fopen(filepath.string().c_str(), "rb");
  if (!file)
    return Result<unsigned char *, int>::ERR(-1);

  if (fseek(file, 0, SEEK_END))
    return Result<unsigned char *, int>::ERR(1);

  size_t fsize = ftell(file);
  fseek(file, 0, SEEK_SET);

  unsigned char *outbuffer;
  if (alloc)
    outbuffer = alloc(allocator, fsize);
  else
    outbuffer = new unsigned char[fsize];

  return Result<unsigned char *, int>::OK(outbuffer);
}

Result<no_error, int> WriteFile(const std::filesystem::path &filepath,
                                const void *data, size_t size) {

  FILE *file = fopen(filepath.string().c_str(), "wb");
  if (!file)
    return Result<no_error, int>::ERR(-1);

  if (fwrite(data, 1, size, file) < size)
    return Result<no_error, int>::ERR(1);

  fclose(file);

  return Result<no_error, int>::OK(false);
}

Result<no_error, int>
WriteFileIfNotExists(const std::filesystem::path &filepath, const void *data,
                     size_t size) {

  FILE *file = fopen(filepath.string().c_str(), "rb");
  if (file) {
    fclose(file);
    return Result<no_error, int>::OK(false);
  }
  fclose(file);
  return WriteFile(filepath, data, size);
}

} // namespace FileUtils
