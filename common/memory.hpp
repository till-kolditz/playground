#pragma once

#include <memory>
#include <memory_resource>

namespace common {

template <typename T> class Deleter {
  std::pmr::memory_resource *m_resource;

public:
  Deleter() = default;

  Deleter(std::pmr::memory_resource *resource) : m_resource(resource) {}

  void operator()(T *obj) const {
    obj->~T();
    m_resource->deallocate(obj, sizeof(T), alignof(T));
  }
};

template <typename T> using UniquePtr = std::unique_ptr<T, Deleter<T>>;

template <typename T>
UniquePtr<T> MakeUnique(std::pmr::memory_resource *resource, T &&obj) {
  void *mem = resource->allocate(sizeof(T), alignof(T));
  return UniquePtr<T>{new (mem) T(std::forward<T>(obj)), Deleter<T>{resource}};
}

template <typename T, typename... Args>
UniquePtr<T> MakeUnique(std::pmr::memory_resource *resource, Args &&...args) {
  auto *mem = static_cast<T *>(resource->allocate(sizeof(T), alignof(T)));
  new (mem) T(std::forward<Args>(args)...);
  return UniquePtr<T>{mem, Deleter<T>{resource}};
}

} // namespace common
