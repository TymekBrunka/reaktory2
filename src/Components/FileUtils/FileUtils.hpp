#pragma once
#include <Errors/Errors.hpp>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <variant>
namespace FileUtils {

template <typename T, typename E> using Result = Errors::Result<T, E>;
using no_error = Errors::no_error;

extern std::filesystem::path HOME_DIR;
extern std::filesystem::path APP_ROOT;

typedef char *(*alloc_fun)(void *alloc, size_t n);
typedef void (*free_fun)(void *alloc, void *buff, size_t n);

struct ReadResult {
  char *data;
  size_t length;
};

Result<ReadResult, int> ReadFilex(const std::filesystem::path &filepath,
                                  alloc_fun alloc = nullptr,
                                  free_fun frre = nullptr,
                                  void *allocator = nullptr);

template <class Allocator = std::allocator<char>>
Result<ReadResult, int>
ReadFile(const std::filesystem::path &filepath,
         const Allocator &alloc = std::allocator<char>()) {

  alloc_fun allo = [](void *aloc, size_t n) {
    return std::allocator_traits<Allocator>::allocate((Allocator &)aloc, n);
  };

  free_fun frre = [](void *aloc, void *buff, size_t n) {
    return std::allocator_traits<Allocator>::deallocate((Allocator &)aloc,
                                                        (char *)buff, n);
  };

  return ReadFilex(filepath, allo, frre, (void *)&alloc);
}

Result<no_error, int> WriteFile(const std::filesystem::path &filepath,
                                const void *data, size_t size);

Result<no_error, int>
WriteFileIfNotExists(const std::filesystem::path &filepath, const void *data,
                     size_t size);

// [ unified io ]
// ------------------------------------------------------------------

class path {
private:
  std::variant<std::filesystem::path, std::string> Path;

public:
  path() = default;
  path(const std::filesystem::path &path_) : Path(path_) {};
  path(const std::string &path_) : Path(path_) {};
  ~path() = default;

  inline std::filesystem::path operator/(const std::filesystem::path &path_) {
    return std::get<std::filesystem::path>(Path) / path_;
  }

  inline std::filesystem::path &operator/=(const std::filesystem::path &path_) {
    return std::get<std::filesystem::path>(Path) /= path_;
  }

  inline path operator/(const path &path_) {
    const std::filesystem::path *paf =
        std::get_if<std::filesystem::path>(&Path);
    const std::filesystem::path *paf_ =
        std::get_if<std::filesystem::path>(&path_.Path);

    const std::string *spaf = std::get_if<std::string>(&Path);
    const std::string *spaf_ = std::get_if<std::string>(&path_.Path);
    if (paf && paf_)
      return path((*paf) / (*paf_));
    else if (spaf && spaf_)
      return path(*spaf + "/" + *spaf_);
  }

  inline path &operator/=(const path &path_) {
    const std::filesystem::path *paf =
        std::get_if<std::filesystem::path>(&Path);
    const std::filesystem::path *paf_ =
        std::get_if<std::filesystem::path>(&path_.Path);

    const std::string *spaf = std::get_if<std::string>(&Path);
    const std::string *spaf_ = std::get_if<std::string>(&path_.Path);
    if (paf && paf_)
      *((std::filesystem::path *)paf) /= *paf_;
    else if (spaf && spaf_)
      *((std::string *)spaf) += "/" + *spaf_;

    return *this;
  }
};

class Fs {
public:
  virtual Result<ReadResult, int> ReadFilex(const path &filepath,
                                            alloc_fun alloc = nullptr,
                                            free_fun frre = nullptr,
                                            void *allocator = nullptr) = 0;

  template <class Allocator = std::allocator<char>>
  Result<ReadResult, int>
  ReadFile(const path &filepath,
           const Allocator &alloc = std::allocator<char>()) {

    alloc_fun allo = [](void *aloc, size_t n) {
      return std::allocator_traits<Allocator>::allocate((Allocator &)aloc, n);
    };

    free_fun frre = [](void *aloc, void *buff, size_t n) {
      return std::allocator_traits<Allocator>::deallocate((Allocator &)aloc,
                                                          (char *)buff, n);
    };

    return ReadFilex(filepath, allo, frre, (void *)&alloc);
  }

  virtual bool FileExists(const path &filepath) = 0;
};

class RealFs : Fs {
};

} // namespace FileUtils
