#pragma once
#include <cstddef> // For std::ptrdiff_t
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <iterator> // For std::forward_iterator_tag
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <variant>
#include <vector>

template <typename> struct TypeIdHack {};

class AnyImplBase {
  friend class Any;

protected:
  virtual void copyFrom(AnyImplBase *src) = 0;
  virtual AnyImplBase *copy() = 0;
  virtual void moveFrom(AnyImplBase *src) = 0;

public:
  virtual ~AnyImplBase() = default;

  virtual void *get() = 0;
  virtual size_t data_type() = 0;

  template <typename T> T *convertTo() { return (T *)this; }
};

template <typename T>
class AnyImpl
    : public AnyImplBase { // public kw is not only required for method
                           // visibility for user but also smart pointers
  friend class Any;

protected:
  T data;

  void copyFrom(AnyImplBase *src) override { data = ((AnyImpl<T> *)src)->data; }

  AnyImplBase *copy() override {
    AnyImpl *a = new AnyImpl;
    a->data = data;
    return a;
  };

  void moveFrom(AnyImplBase *src) override {
    data = std::move(((AnyImpl<T> *)src)->data);
  }

public:
  AnyImpl() = default;
  ~AnyImpl() override = default;

  AnyImpl(const T &that) { data = that; }
  AnyImpl &operator=(const T &that) { data = that; }
  AnyImpl(T &&that) { data = that; }
  AnyImpl &operator=(T &&that) { data = that; }

  T &operator()() const { return data; }
  T &&operator()() { return std::move(data); }

  void *get() override { return &data; }
  size_t data_type() override { return typeid(TypeIdHack<T>).hash_code(); }
};

class Any {
public:
  using uptr_t = std::unique_ptr<AnyImplBase>;
  using sptr_t = std::shared_ptr<AnyImplBase>;

  struct None {
    bool dummy = false;

    constexpr None() : dummy(false) {}
  };

  struct uptr_wrapper {
    uptr_t self;

    uptr_wrapper() = default;
    ~uptr_wrapper() = default;

    template <typename T> uptr_wrapper() {
      self = std::make_unique<AnyImpl<T>>();
    }

    inline uptr_wrapper &operator=(const uptr_wrapper &other) {
      if (this != &other) {
        if (other.self.get()->data_type() == self.get()->data_type()) {
          self.get()->copyFrom(other.self.get());
        } else {
          self.reset(other.self.get()->copy());
        }
      }
      return *this;
    }

    inline uptr_wrapper(const uptr_wrapper &other) { *this = other; }

    inline uptr_wrapper &operator=(uptr_wrapper &&other) {
      if (this != &other)
        self = std::move(other.self);
      return *this;
    }

    inline uptr_wrapper(uptr_wrapper &&other) { *this = std::move(other); }
  };

  using variant_t =
      std::variant<None, bool, int, float, void *, uptr_wrapper, sptr_t>;
  variant_t data;

  constexpr Any() : data(None{}) {}
  Any(const variant_t &data_) : data(data_) {}
  Any(variant_t &&data_) : data(std::move(data_)) {}
  ~Any() = default;

  template <typename T> Any(const T &data_) {
    auto s = std::make_shared<AnyImpl<T>>();
    s.get()->data = data_;
    data = std::move(s);
  }

  template <typename T> Any(T &&data_) noexcept {
    auto s = std::make_shared<AnyImpl<T>>();
    s.get()->data = std::move(data_);
    data = std::move(s);
  }

  inline Any(Any &other) { data = other.data; }
  inline Any(const Any &other) { data = other.data; }
  inline Any(Any &&other) { data = std::move(other.data); }

  Any &operator=(const Any &other);

  Any &operator=(Any &&other) noexcept;

  inline bool is_none() { return std::get_if<None>(&data); }

  template <typename T> T *get() {
    if (sptr_t *sptr = (sptr_t *)std::get_if<sptr_t>(&data)) {
      return sptr->get()->data_type() == typeid(TypeIdHack<T>).hash_code()
                 ? (T *)sptr->get()->get()
                 : nullptr;
    }
    return nullptr;
  }

  template <typename T> T *get_shared() {
    if (sptr_t *sptr = (sptr_t *)std::get_if<sptr_t>(&data)) {
      return sptr->get()->data_type() == typeid(TypeIdHack<T>).hash_code()
                 ? (T *)sptr->get()->get()
                 : nullptr;
    }
    return nullptr;
  }

  template <typename T> T get_unique() {
    if (uptr_t *uptr = (uptr_t *)std::get_if<uptr_wrapper>(&data)) {
      return uptr->get()->data_type() ==
                     typeid(TypeIdHack<std::remove_pointer<T>>).hash_code()
                 ? (T)uptr->get()->get()
                 : nullptr;
    }
    return nullptr;
  }

  template <typename T> T get_either() {
    T *ptr = get<T>();
    if (!ptr) {
      ptr = get_unique<T>();
      return ptr ? ptr : get_shared<T>();
    }
    return ptr;
  }
};

inline Any &Any::operator=(const Any &other) {
  if (this != &other) {
    data = other.data;
  }
  return *this;
}

inline Any &Any::operator=(Any &&other) noexcept {
  if (this != &other) {
    data = std::move(other.data);
  }
  return *this;
}

template <> inline Any::Any(const Any &other) { data = other.data; }

template <> inline Any::Any(Any &&other) noexcept {
  data = std::move(other.data);
}

template <> inline bool *Any::get() {
  return (bool *)std::get_if<bool>(&data);
};

template <> inline int *Any::get() { return (int *)std::get_if<int>(&data); };

template <> inline float *Any::get() {
  return (float *)std::get_if<float>(&data);
};

template <> inline void **Any::get() {
  return (void **)std::get_if<void *>(&data);
};

template <> inline Any::Any(const bool &x) { data = x; }
template <> inline Any::Any(bool &&x) noexcept { data = x; }

template <> inline Any::Any(const int &x) { data = x; }
template <> inline Any::Any(int &&x) noexcept { data = x; }

template <> inline Any::Any(const float &x) { data = x; }
template <> inline Any::Any(float &&x) noexcept { data = x; }

typedef void *voidptr;

template <> inline Any::Any(const voidptr &x) { data = (void *)x; }
template <> inline Any::Any(voidptr &&x) noexcept { data = x; }

struct tHandle {
  uint8_t gen = 0;
  int16_t idx = -1;
};

template <typename T> class Pool {
private:
  struct Index {
    uint8_t gen = 0;
    int16_t idx = -1;
  };

  std::vector<Index> indices; // index -> element
  std::vector<T> elements;
  std::vector<int16_t> element_to_index; // element -> index
  std::vector<int16_t>
      free_list; // stores indices to indices array, that became empty, elements
                 // are removed from this list when new element is being added

public:
  Pool() = default;
  ~Pool() = default;

  int16_t size() { return elements.size(); }
  int16_t idx_size() { return indices.size(); }

  std::vector<T>::iterator begin() { return elements.begin(); }
  std::vector<T>::const_iterator begin() const { return elements.begin(); }

  std::vector<T>::iterator end() { return elements.end(); }
  std::vector<T>::const_iterator end() const { return elements.end(); }

  std::vector<T>::iterator rbegin() { return elements.rbegin(); }
  std::vector<T>::const_iterator rbegin() const { return elements.rbegin(); }

  std::vector<T>::iterator rend() { return elements.rend(); }
  std::vector<T>::const_iterator rend() const { return elements.rend(); }

  std::vector<Index> &get_indices() { return indices; }

  std::vector<T> &expose() { return elements; }

  T *get(tHandle handle, bool unsafe = false) {
    if (handle.idx < 0 || handle.idx >= (int16_t)indices.size())
      return nullptr;

    Index *idx = &indices[handle.idx];
    return idx->idx != -1 && idx->idx < elements.size() &&
                   (unsafe || idx->gen == handle.gen)
               ? &elements[idx->idx]
               : nullptr;
  }

  tHandle add(T &&element) {
    if (!free_list.empty()) {
      int16_t freeidx = free_list[free_list.size() - 1];
      free_list.pop_back();
      Index *idx = &indices[freeidx];

      if (idx->idx != -1)
        return tHandle{.gen = 0, .idx = -2}; // possibly crash program when
                                             // pool's structure gets corrupted

      idx->idx = elements.size();
      elements.push_back(std::move(element));
      element_to_index.push_back(freeidx);
      return tHandle{.gen = ++idx->gen, .idx = freeidx};
    }

    indices.push_back(Index{.gen = 1, .idx = (int16_t)elements.size()});
    elements.push_back(std::move(element));
    element_to_index.push_back(indices.size() - 1);
    return tHandle{.gen = 1, .idx = (int16_t)(indices.size() - 1)};
  }

  tHandle add(const T &element) {
    T el = element;
    return add(std::move(el));
  }

  void remove(tHandle handle, bool unsafe = false) {
    if (handle.idx < 0 || handle.idx >= indices.size())
      return;

    Index *idx = &indices[handle.idx];
    if (idx->idx == -1 || idx->idx >= elements.size() ||
        (!unsafe && idx->gen != handle.gen))
      return;

    free_list.push_back(handle.idx);

    if (elements.size() - 1 != idx->idx) {
      int16_t index_to_last_element =
          element_to_index[element_to_index.size() - 1];
      indices[index_to_last_element].idx = idx->idx;
      element_to_index[idx->idx] =
          index_to_last_element; // im surprised how long it took me to find out
                                 // that i forgot to write this line

      T last_element = std::move(elements[elements.size() - 1]);
      elements[idx->idx] = std::move(last_element);
    }

    element_to_index.pop_back();
    elements.pop_back();
    idx->idx = -1;
  }
};
