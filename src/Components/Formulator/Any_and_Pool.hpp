#pragma once
#include <cstddef> // For std::ptrdiff_t
#include <cstdint>
#include <functional>
#include <iterator> // For std::forward_iterator_tag
#include <type_traits>
#include <typeinfo>
#include <vector>

template <typename> struct TypeIdHack {};

class Any {
  union {
    bool Bool;
    int Int;
    float Float;
    void *Ptr;
  } data;
  size_t type_hash = 0; // from typeid().hash_code()
  std::function<void(void *)> destructor;

public:
  inline Any() { data.Ptr = nullptr; }
  inline ~Any() {
    if (data.Ptr)
      destructor(data.Ptr);
  }

  Any(const Any &other) = delete;
  Any &operator=(const Any &other) = delete;

  inline Any(Any &&other) {
    if (destructor)
      destructor(data.Ptr);
    data = other.data;
    type_hash = other.type_hash;
    destructor = other.destructor;
    other.data.Ptr = nullptr;
    other.type_hash = 0;
    other.destructor = std::function<void(void *)>{};
  }

  inline Any &operator=(Any &&other) {
    if (this != &other) {
      if (destructor)
        destructor(data.Ptr);
      data = other.data;
      type_hash = other.type_hash;
      destructor = other.destructor;
      other.data.Ptr = nullptr;
      other.type_hash = 0;
      other.destructor = std::function<void(void *)>{};
    }
    return *this;
  }

  template <typename T> bool is_type() {
    return type_hash == typeid(TypeIdHack<T>).hash_code();
  }

  template <typename T> bool is_same_type(const T &other) {
    return type_hash == typeid(TypeIdHack<T>).hash_code();
  }

private:
  // template <typename T> void put_base(const T &value);

  template <typename T> void put_base(const T &value) {
    *((T *)data.Ptr) = value;
    type_hash = typeid(TypeIdHack<T>).hash_code();
    destructor = [](void *ptr) { delete (T *)ptr; };
  }

  template <typename T> void put_base(T &&value) {
    *((T *)data.Ptr) = value;
    type_hash = typeid(TypeIdHack<T>).hash_code();
    destructor = [](void *ptr) { delete (T *)ptr; };
  }

  template <typename T> T *get_base() { return (T *)data.Ptr; }

public:
  template <typename T> void put(const T &value) {
    if (!is_type<T>()) {
      destructor(data.Ptr);
      data.Ptr = new T();
    }
    put_base<T>(value);
    type_hash = typeid(TypeIdHack<bool>).hash_code();
  }

  template <typename T> void put(T &&value) {
    if (!is_type<T>()) {
      destructor(data.Ptr);
      data.Ptr = new T();
    }
    put_base<T>(value);
    type_hash = typeid(TypeIdHack<bool>).hash_code();
  }

  template <typename T> T *get() {
    if (!is_type<T>())
      return nullptr;
    return get_base<T>();
  }
};

template <> inline void Any::put_base<bool>(const bool &value) {
  data.Bool = value;
  destructor = std::function<void(void *)>{};
}

template <> inline void Any::put_base<int>(const int &value) {
  data.Int = value;
  destructor = std::function<void(void *)>{};
}

template <> inline void Any::put_base<float>(const float &value) {
  data.Float = value;
  destructor = std::function<void(void *)>{};
}

template <> inline void Any::put_base<bool>(bool &&value) {
  data.Bool = value;
  destructor = std::function<void(void *)>{};
}

template <> inline void Any::put_base<int>(int &&value) {
  data.Int = value;
  destructor = std::function<void(void *)>{};
}

template <> inline void Any::put_base<float>(float &&value) {
  data.Float = value;
  destructor = std::function<void(void *)>{};
}

template <> inline bool *Any::get_base() { return &data.Bool; };
template <> inline int *Any::get_base() { return &data.Int; }
template <> inline float *Any::get_base() { return &data.Float; }

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

  T *get(tHandle handle) {
    if (handle.idx < 0 || handle.idx >= (int16_t)indices.size())
      return nullptr;

    Index *idx = &indices[handle.idx];
    return idx->idx != -1 && idx->gen == handle.gen ? &elements[idx->idx]
                                                    : nullptr;
  }

  tHandle add(const T &element) {
    if (!free_list.empty()) {
      int16_t freeidx = free_list[free_list.size() - 1];
      free_list.pop_back();
      Index *idx = &indices[freeidx];
      idx->idx = elements.size();
      elements.push_back(element);
      element_to_index.push_back(freeidx);
      return tHandle{.gen = idx->gen++, .idx = freeidx};
    }

    indices.push_back(Index{.gen = 1, .idx = (int16_t)elements.size()});
    elements.push_back(element);
    element_to_index.push_back(indices.size() - 1);
    return tHandle{.gen = 1, .idx = (int16_t)(indices.size() - 1)};
  }

  void remove(tHandle handle) {
    if (handle.idx < 0 || handle.idx >= indices.size())
      return;

    Index *idx = &indices[handle.idx];
    if (idx->idx == -1 || idx->gen != handle.gen)
      return;

    free_list.push_back(handle.idx);
    int16_t index_to_last_element =
        element_to_index[element_to_index.size() - 1];
    indices[index_to_last_element].idx = idx->idx;
    element_to_index.pop_back();

    T last_element = std::move(elements[elements.size() - 1]);
    elements[idx->idx] = std::move(last_element);
    elements.pop_back();
    idx->idx = -1;
  }
};
