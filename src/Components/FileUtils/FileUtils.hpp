#pragma once
#include <Errors/Errors.hpp>
#include <cstddef>
#include <filesystem>
#include <fstream>
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

  inline std::filesystem::path to_fs() const {
    if (const std::filesystem::path *paf =
            std::get_if<std::filesystem::path>(&Path)) {
      return *paf;
    } else {
      return std::filesystem::path{std::get<std::string>(Path)};
    }
  }

  inline operator std::filesystem::path() const { return std::move(to_fs()); }

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

  inline path filename() const {
    if (const std::filesystem::path *paf =
            std::get_if<std::filesystem::path>(&Path)) {
      std::filesystem::path paf_ = *paf;
      return path{paf_.filename()};
    } else {
      const std::string &s = std::get<std::string>(Path);
      int idx = s.find_last_of('/');
      return path{s.substr(idx, s.size() - idx)};
    }
  }
};

class FsStream {
public:
  inline virtual ~FsStream() {}
  void *file = nullptr;

  virtual size_t Read(void *out, size_t size) = 0;
  virtual size_t Write(const void *data, size_t size) = 0;
  virtual bool Seek(size_t offset, bool at_the_end) = 0;
  virtual size_t Tell() const = 0;
  virtual size_t FileSize() const = 0;
  // void Flush () { ... }
};

class Fs {
public:
  virtual char Separator() const = 0;

  virtual bool FileExists(const path &filepath) const = 0;

  virtual FsStream *Open(const path &path, const char *mode) = 0;

  virtual void Close(FsStream *file) = 0;

  inline Result<ReadResult, int> ReadFilex(const path &filepath,
                                           alloc_fun alloc = nullptr,
                                           free_fun frre = nullptr,
                                           void *allocator = nullptr) {
    if (!FileExists(filepath))
      return Result<ReadResult, int>::ERR(-2);

    FsStream *file = Open(filepath, "rb");
    if (!file)
      return Result<ReadResult, int>::ERR(-1);

    size_t fsize = file->FileSize();
    char *outbuffer;
    if (alloc)
      outbuffer = alloc(allocator, fsize);
    else
      outbuffer = new char[fsize];
    outbuffer[fsize] = '\0';

    if (file->Read(outbuffer, fsize) < fsize) {
      if (frre)
        frre(outbuffer, allocator, fsize);
      else
        delete[] outbuffer;
      return Result<ReadResult, int>::ERR(1);
    }

    return Result<ReadResult, int>::OK(
        ReadResult{.data = outbuffer, .length = fsize});
  }

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
};

class RealFsStream : public FsStream {
public:
  inline ~RealFsStream() {
    if (file)
      // ((std::fstream *)file)->~fstream();
      delete (std::fstream *)file;
  };

  RealFsStream() = default;

  RealFsStream(std::fstream *stream) { file = stream; }

  RealFsStream(const RealFsStream &other) = delete;

  RealFsStream &operator=(const RealFsStream &other) = delete;

  inline RealFsStream(RealFsStream &&other) {
    file = other.file;
    other.file = nullptr;
  }

  inline RealFsStream &operator=(RealFsStream &other) {
    if (this != &other) {
      file = other.file;
      other.file = nullptr;
    }
    return *this;
  }

  inline size_t Read(void *out, size_t size) {
    ((std::fstream *)file)->read((char *)out, size);
    size_t read = ((std::fstream *)file)->gcount();
    return read;
    // return size;
  }

  inline size_t Write(const void *data, size_t size) {
    ((std::fstream *)file)->write((char *)data, size);
    size_t written = ((std::fstream *)file)->gcount();
    return written;
    // return size;
  }

  inline bool Seek(size_t offset, bool at_the_end) {
    try {
      ((std::fstream *)file)
          ->seekg(offset, at_the_end ? std::ios_base::end : std::ios_base::beg);
      return true;
    } catch (const std::ios_base::failure &err) {
      return false;
    }
  }

  inline size_t Tell() const {
    try {
      size_t t = ((std::fstream *)file)->tellg();
      return t;
    } catch (const std::ios_base::failure &err) {
      return 0;
    }
  }

  inline size_t FileSize() const {
    RealFsStream *dis = (RealFsStream *)this;
    size_t t = dis->Tell();
    if (!dis->Seek(0, true))
      return 0;
    size_t filesize = dis->Tell();
    dis->Seek(t, false);
    return filesize;
  };
  // void Flush () { ... }
};

class RealFs : public Fs {
private:
  std::filesystem::path root;

public:
  RealFs() = default;
  RealFs(const std::filesystem::path &path) : root(path) {};
  RealFs(std::filesystem::path &&path) : root(path) {};
  ~RealFs() = default;

  inline char Separator() const override {
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
  }

  inline FsStream *Open(const path &path, const char *mode) override {
    if (!FileExists(path))
      return nullptr;

    std::fstream *stream =
        new std::fstream{path.to_fs(), std::ios_base::in | std::ios_base::out |
                                           std::ios_base::binary};

    if (!stream->is_open()) {
      delete stream;
      return nullptr;
    }

    RealFsStream *file = new RealFsStream{stream};
    return file;
  }

  inline void Close(FsStream *file) override {
    ((std::fstream *)file->file)->close();
  }

  inline bool FileExists(const path &filepath) const override {
    return std::filesystem::exists(root / filepath) &&
           std::filesystem::is_regular_file(root / filepath);
  };
};

} // namespace FileUtils
