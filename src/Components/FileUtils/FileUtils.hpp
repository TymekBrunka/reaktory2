#pragma once
#include <Errors/Errors.hpp>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <utility>
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
  path(const char *path_) : Path(std::string(path_)) {};
  ~path() = default;

  inline operator std::filesystem::path() const {
    if (const std::filesystem::path *paf =
            std::get_if<std::filesystem::path>(&Path)) {
      return *paf;
    } else {
      return std::get<std::string>(Path);
    }
  }

  inline operator std::filesystem::path &() const {
    return *const_cast<std::filesystem::path *>(
        &std::get<std::filesystem::path>(Path));
  }

  inline operator std::string &() const {
    return *const_cast<std::string *>(&std::get<std::string>(Path));
  }

  inline std::string to_string() const {
    if (const std::filesystem::path *path_ =
            std::get_if<std::filesystem::path>(&Path)) {
      return path_->string();
    } else {
      return std::get<std::string>(Path);
    }
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
    else if (paf && spaf_)
      return path(*paf / *spaf_);
    else
      std::unreachable();
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
    else if (paf && spaf_)
      *((std::filesystem::path *)paf) /= *spaf_;

    return *this;
  }

  inline path folder() const {
    if (const std::filesystem::path *paf =
            std::get_if<std::filesystem::path>(&Path)) {
      std::filesystem::path paf_ = *paf;
      paf_.remove_filename();
      return path{paf_};
    } else {
      const std::string &s = std::get<std::string>(Path);
      return path{s.substr(0, s.find_last_of('/'))};
    }
  }
};

class Fs {
public:
  virtual char separator() = 0;

  virtual Result<ReadResult, int>
  ReadFilex(const path &filepath, alloc_fun alloc = nullptr,
            free_fun frre = nullptr, void *allocator = nullptr) const = 0;

  template <class Allocator = std::allocator<char>>
  Result<ReadResult, int>
  ReadFile(const path &filepath,
           const Allocator &alloc = std::allocator<char>()) const {

    alloc_fun allo = [](void *aloc, size_t n) {
      return std::allocator_traits<Allocator>::allocate((Allocator &)aloc, n);
    };

    free_fun frre = [](void *aloc, void *buff, size_t n) {
      return std::allocator_traits<Allocator>::deallocate((Allocator &)aloc,
                                                          (char *)buff, n);
    };

    return ReadFilex(filepath, allo, frre, (void *)&alloc);
  }

  virtual bool FileExists(const path &filepath) const = 0;
};

class RealFs : public Fs {
private:
  std::filesystem::path root;

public:
  RealFs() = default;
  RealFs(const std::filesystem::path &path) : root(path) {};
  RealFs(std::filesystem::path &&path) : root(path) {};
  ~RealFs() = default;

  inline char separator() override {
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
  }

  inline Result<ReadResult, int>
  ReadFilex(const path &filepath, alloc_fun alloc = nullptr,
            free_fun frre = nullptr, void *allocator = nullptr) const override {

    return FileUtils::ReadFilex(root / filepath, alloc, frre, allocator);
  };

  inline bool FileExists(const path &filepath) const override {
    return std::filesystem::exists(root / filepath) &&
           std::filesystem::is_regular_file(root / filepath);
  };
};

} // namespace FileUtils
