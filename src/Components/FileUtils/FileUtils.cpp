#include <FileUtils.hpp>
#include <cstddef>
#include <fstream>
namespace FileUtils {

std::filesystem::path HOME_DIR = "";
std::filesystem::path APP_ROOT = "";

Result<ReadResult, int> ReadFilex(const std::filesystem::path &filepath,
                                  alloc_fun alloc, free_fun frre,
                                  void *allocator) {

  if (!std::filesystem::exists(filepath))
    return Result<ReadResult, int>::ERR(-2);

  std::ifstream file(filepath, std::ios_base::in | std::ios_base::binary);
  if (!file.is_open())
    return Result<ReadResult, int>::ERR(-1);

  file.seekg(0, std::ios_base::end);
  size_t fsize = file.tellg();
  file.seekg(0, std::ios_base::beg);

  char *outbuffer;
  if (alloc)
    outbuffer = alloc(allocator, fsize);
  else
    outbuffer = new char[fsize];

  file.read(outbuffer, fsize);
  if (file.gcount() < fsize) {
    if (frre)
      frre(outbuffer, allocator, fsize);
    else
      delete[] outbuffer;
    return Result<ReadResult, int>::ERR(1);
  }

  return Result<ReadResult, int>::OK(
      ReadResult{.data = outbuffer, .length = fsize});
}

Result<no_error, int> WriteFile(const std::filesystem::path &filepath,
                                const void *data, size_t size) {

  std::ofstream file(filepath, std::ios_base::out | std::ios_base::binary);
  if (!file.is_open())
    return Result<no_error, int>::ERR(-1);

  try {
    file.write((const char *)data, size);
  } catch (std::exception &err) {
    file.close();
    return Result<no_error, int>::ERR(1);
  }

  file.close();
  return Result<no_error, int>::OK(false);
}

Result<no_error, int>
WriteFileIfNotExists(const std::filesystem::path &filepath, const void *data,
                     size_t size) {

  std::ifstream file(filepath, std::ios_base::in | std::ios_base::binary);
  if (file.is_open()) {
    file.close();
    return Result<no_error, int>::OK(false);
  }
  file.close();
  return WriteFile(filepath, data, size);
}

} // namespace FileUtils
