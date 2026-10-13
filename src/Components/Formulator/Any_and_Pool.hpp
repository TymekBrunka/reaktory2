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
#include <vector>

template <typename> struct TypeIdHack {};

// class Any {
//   union {
//     bool Bool;
//     int Int;
//     float Float;
//     void *Ptr;
//   } data;
//   size_t type_hash = 0; // from typeid().hash_code()
//   std::function<void(void *)> destructor;
//   std::function<void(void *, void *)> copy_assignment;
//
// public:
//   inline Any() { data.Ptr = nullptr; }
//   inline ~Any() {
//     if (data.Ptr)
//       destructor(data.Ptr);
//   }
//
//   Any(const Any &other) = delete;
//   Any &operator=(const Any &other) = delete;
//
//   inline Any(const Any &other) {
//     if (destructor && type_hash != other.type_hash)
//       destructor(data.Ptr);
//     data = other.data;
//     type_hash = other.type_hash;
//     destructor = other.destructor;
//     copy_assignment = other.copy_assignment;
//   }
//
//   inline Any(Any &&other) {
//     if (destructor && type_hash != other.type_hash)
//       destructor(data.Ptr);
//     data = other.data;
//     type_hash = other.type_hash;
//     destructor = other.destructor;
//     copy_assignment = other.copy_assignment;
//     other.data.Ptr = nullptr;
//     other.type_hash = 0;
//     other.destructor = std::function<void(void *)>{};
//     other.copy_assignment = std::function<void(void *, void *)>{};
//   }
//
//   inline Any &operator=(Any &&other) {
//     if (this != &other) {
//       if (destructor)
//         destructor(data.Ptr);
//       data = other.data;
//       type_hash = other.type_hash;
//       destructor = other.destructor;
//       copy_assignment = other.copy_assignment;
//       other.data.Ptr = nullptr;
//       other.type_hash = 0;
//       other.destructor = std::function<void(void *)>{};
//       other.copy_assignment = std::function<void(void *, void *)>{};
//     }
//     return *this;
//   }
//
//   template <typename T> bool is_type() {
//     return type_hash == typeid(TypeIdHack<T>).hash_code();
//   }
//
//   template <typename T> bool is_same_type(const T &other) {
//     return type_hash == typeid(TypeIdHack<T>).hash_code();
//   }
//
// private:
//   // template <typename T> void put_base(const T &value);
//
//   template <typename T> void put_base(const T &value) {
//     *((T *)data.Ptr) = value;
//     type_hash = typeid(TypeIdHack<T>).hash_code();
//     destructor = [](void *ptr) { delete (T *)ptr; };
//     copy_assignment = [](void *dest, void *src) { *(T *)dest = *(T *)src; }
//   }
//
//   template <typename T> void put_base(T &&value) {
//     *((T *)data.Ptr) = value;
//     type_hash = typeid(TypeIdHack<T>).hash_code();
//     destructor = [](void *ptr) { delete (T *)ptr; };
//     copy_assignment = [](void *dest, void *src) { *(T *)dest = *(T *)src; }
//   }
//
//   template <typename T> T *get_base() { return (T *)data.Ptr; }
//
// public:
//   template <typename T> void put(const T &value) {
//     if (!is_type<T>()) {
//       destructor(data.Ptr);
//       data.Ptr = new T();
//     }
//     put_base<T>(value);
//     type_hash = typeid(TypeIdHack<bool>).hash_code();
//   }
//
//   template <typename T> void put(T &&value) {
//     if (!is_type<T>()) {
//       destructor(data.Ptr);
//       data.Ptr = new T();
//     }
//     put_base<T>(value);
//     type_hash = typeid(TypeIdHack<bool>).hash_code();
//   }
//
//   template <typename T> T *get() {
//     if (!is_type<T>())
//       return nullptr;
//     return get_base<T>();
//   }
// };
//
// template <> inline void Any::put_base<bool>(const bool &value) {
//   data.Bool = value;
//   destructor = std::function<void(void *)>{};
//   copy_assignment = std::function<void(void *, void *)>{};
// }
//
// template <> inline void Any::put_base<int>(const int &value) {
//   data.Int = value;
//   destructor = std::function<void(void *)>{};
//   copy_assignment = std::function<void(void *, void *)>{};
// }
//
// template <> inline void Any::put_base<float>(const float &value) {
//   data.Float = value;
//   destructor = std::function<void(void *)>{};
//   copy_assignment = std::function<void(void *, void *)>{};
// }
//
// template <> inline void Any::put_base<bool>(bool &&value) {
//   data.Bool = value;
//   destructor = std::function<void(void *)>{};
//   copy_assignment = std::function<void(void *, void *)>{};
// }
//
// template <> inline void Any::put_base<int>(int &&value) {
//   data.Int = value;
//   destructor = std::function<void(void *)>{};
//   copy_assignment = std::function<void(void *, void *)>{};
// }
//
// template <> inline void Any::put_base<float>(float &&value) {
//   data.Float = value;
//   destructor = std::function<void(void *)>{};
//   copy_assignment = std::function<void(void *, void *)>{};
// }
//
// template <> inline bool *Any::get_base() { return &data.Bool; };
// template <> inline int *Any::get_base() { return &data.Int; }
// template <> inline float *Any::get_base() { return &data.Float; }
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
  enum class Tag : uint8_t {
    NONE = 0,
    BOOL,
    INT,
    FLOAT,
    PTR,
    UPTR,
    SPTR,
    COUNT,
  };

  union Primitives {
    bool Bool;
    int Int;
    float Float;
  };

private:
  struct Buffer {
    uint8_t buf[sizeof(std::shared_ptr<AnyImplBase>)];

    inline Buffer() { memset(buf, 0, sizeof(std::shared_ptr<AnyImplBase>)); }
  };

  union {
    void *Ptr;
    std::unique_ptr<AnyImplBase> Uptr;
    std::shared_ptr<AnyImplBase> Sptr;
    Buffer Buf;
  };

  inline bool is_little_endian() {
    static const uint8_t test[2] = {1, 2};
    if (*(uint16_t *)test == 513) // 2 * 256^1 + 1 * 256^0
      return true;
    return false;
  }

  inline Tag *tag_offset() {
    return is_little_endian() ? (Tag *)((uint8_t *)&Ptr + 7) : (Tag *)&Ptr;
  };

  inline Primitives *data_offset() {
    return is_little_endian() ? (Primitives *)&Ptr
                              : (Primitives *)((uint8_t *)&Ptr + 2);
  };

  inline Tag getTag() { return *tag_offset(); }
  inline void clearTag() { *tag_offset() = Tag::NONE; }
  inline void setTag(Tag tag) {
    *tag_offset() = tag;
    // if (tag == Tag::SPTR) {
    //   std::cerr << "here\n";
    // }
  }

  inline void clear_old() {
    Tag tag1 = getTag();
    std::cerr << "odl tag: " << (int)tag1 << "\n";
    if (tag1 == Tag::SPTR) {
      std::cerr << "used to be around here\n";
    }

    if ((int)tag1 >= (int)Tag::COUNT) {
      std::cerr << "oh noes\n";
      Buf = Buffer{};
    }

    clearTag();
    if (tag1 == Tag::UPTR) {
      Uptr.~unique_ptr<AnyImplBase>();
      Buf = Buffer{};
      std::cerr << "cleared unique ptr\n";
    } else if (tag1 == Tag::SPTR) {
      Sptr.~shared_ptr<AnyImplBase>();
      Buf = Buffer{};
      std::cerr << "cleared smart ptr\n";
    }
  }

public:
  inline Any() { Buf = Buffer{}; }

  inline Any(bool x) {
    data_offset()->Bool = x;
    setTag(Tag::BOOL);
  }

  inline Any(int x) {
    data_offset()->Int = x;
    setTag(Tag::INT);
  }

  inline Any(float x) {
    data_offset()->Float = x;
    setTag(Tag::FLOAT);
  }

  inline Any(void *x) {
    Ptr = x;
    setTag(Tag::PTR);
  }

  inline Any(std::shared_ptr<AnyImplBase> x) {
    Sptr = std::move(x);
    setTag(Tag::SPTR);
  }

  inline ~Any() { clear_old(); }

  inline Any &operator=(Any &&other) {
    if (this != &other) {
      clear_old();
      Buf = other.Buf;
      other.Buf = Buffer{};
    }
    return *this;
  }

  inline Any(Any &&other) { *this = std::move(other); }

  template <typename T> Any &set_as_uptr(const T &element) {
    clear_old();
    Uptr = std::make_unique<AnyImpl<T>>();
    Uptr.get()->convertTo<AnyImpl<T>>()->data = element;
    setTag(Tag::UPTR);
    return *this;
  }

  template <typename T> Any &set_as_uptr(T &&element) {
    clear_old();
    Uptr = std::make_unique<AnyImpl<T>>();
    Uptr.get()->convertTo<AnyImpl<T>>()->data = std::move(element);
    setTag(Tag::UPTR);
    return *this;
  }

  template <typename T> Any &set_as_shared(const T &element) {
    clear_old();
    Sptr = std::make_shared<AnyImpl<T>>();
    Sptr.get()->convertTo<AnyImpl<T>>()->data = element;
    setTag(Tag::SPTR);
    return *this;
  }

  template <typename T> Any &set_as_shared(T &&element) {
    clear_old();
    Sptr = std::make_shared<AnyImpl<T>>();
    Sptr.get()->convertTo<AnyImpl<T>>()->data = std::move(element);
    setTag(Tag::SPTR);
    return *this;
  }

  inline Any &operator=(const Any &other_) {
    Buffer *buf = (Buffer *)&other_.Buf;
    Any &other = *(Any *)buf;
    if (this != &other_) {
      Tag tag1 = getTag();
      Tag tag2 = other.getTag();

      if (tag1 == Tag::UPTR && tag2 == Tag::UPTR) {
        clearTag();
        other.clearTag();
        if (Uptr.get()->data_type() == other.Uptr.get()->data_type())
          Uptr.get()->copyFrom(other.Uptr.get());
        else {
          Uptr.~unique_ptr();
          Uptr.reset(other.Uptr.get()->copy());
        }
        setTag(tag1);
        other.setTag(tag2);
      }

      else if (tag1 == Tag::SPTR && tag2 == Tag::SPTR) {
        clearTag();
        other.clearTag();
        Sptr.~shared_ptr();
        Sptr = other.Sptr;
        // Sptr.release();
        // Sptr.reset(other.Sptr.get()->copy());
        setTag(tag1);
        other.setTag(tag2);
      }

      else {
        clear_old();
        if (tag2 == Tag::UPTR) {
          other.clearTag();
          Uptr.~unique_ptr();
          Uptr.reset(other.Uptr.get()->copy());
          setTag(Tag::UPTR);
          other.setTag(tag2);
        } else if (tag2 == Tag::SPTR) {
          other.clearTag();
          Sptr.~shared_ptr();
          Sptr = other.Sptr;
          setTag(Tag::SPTR);
          other.setTag(tag2);
        } else
          Buf = other.Buf;
      }
    }
    return *this;
  }

  inline Any(const Any &other) { *this = other; }

  template <typename T> Any(const T &element) { set_as_shared(element); }

  template <typename T> bool holds_ptr_of_type() {
    Tag tag1 = getTag();
    bool ret = false;

    if (tag1 == Tag::UPTR) {
      clearTag();
      ret = Uptr.get()->data_type() == typeid(TypeIdHack<T>).hash_code();
      setTag(tag1);
    }

    else if (tag1 == Tag::SPTR) {
      clearTag();
      ret = Sptr.get()->data_type() == typeid(TypeIdHack<T>).hash_code();
      setTag(tag1);
    }

    return ret;
  }

  template <typename T> T *get() {
    Tag tag1 = getTag();
    T *ret = nullptr;

    if (tag1 == Tag::UPTR) {
      clearTag();
      ret = Uptr.get()->data_type() == typeid(TypeIdHack<T>).hash_code()
                ? (T *)Uptr.get()->get()
                : nullptr;
      setTag(tag1);
    }

    else if (tag1 == Tag::SPTR) {
      clearTag();
      ret = Sptr.get()->data_type() == typeid(TypeIdHack<T>).hash_code()
                ? (T *)Sptr.get()->get()
                : nullptr;
      setTag(tag1);
    }

    return ret;
  }

  inline void setPrimitive(bool x) {
    clear_old();
    setTag(Tag::BOOL);
    data_offset()->Bool = x;
  }

  inline void setPrimitive(int x) {
    clear_old();
    setTag(Tag::INT);
    data_offset()->Int = x;
  }

  inline void setPrimitive(float x) {
    clear_old();
    setTag(Tag::FLOAT);
    data_offset()->Float = x;
  }

  inline void setPrimitive(void *x) {
    clear_old();
    setTag(Tag::PTR);
    Ptr = x;
  }
};

template <> inline bool *Any::get() {
  return getTag() == Tag::BOOL ? &data_offset()->Bool : nullptr;
}

template <> inline int *Any::get() {
  return getTag() == Tag::INT ? &data_offset()->Int : nullptr;
}

template <> inline float *Any::get() {
  return getTag() == Tag::FLOAT ? &data_offset()->Float : nullptr;
}

template <> inline void *Any::get() {
  Tag tag1 = getTag();
  void *ret = nullptr;

  if (tag1 == Tag::PTR) {
    clearTag();
    ret = Ptr;
    setTag(tag1);
  }

  return ret;
}

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
