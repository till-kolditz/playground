#pragma once

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory_resource>
#include <system_error>
#include <unordered_map>
#include <utility>

#include <memory.hpp>

namespace objectstore {

using common::UniquePtr;

class Object {
  virtual std::iostream *stream() = 0;

  virtual std::pair<std::uintmax_t, std::error_code> size() const = 0;
};

class StoredObject : public Object {
public:
  virtual ~StoredObject() = default;

  virtual void open() = 0;
  virtual void close() = 0;
  virtual void destroy() = 0;
  virtual bool exists() const = 0;
};

class StoredFile : public StoredObject {
  std::pmr::memory_resource *m_resource;
  std::filesystem::path m_file_path;
  mutable std::fstream m_stream;

public:
  StoredFile(std::pmr::memory_resource *resource,
             std::filesystem::path file_path)
      : m_resource{resource}, m_file_path{std::move(file_path)} {}

  StoredFile(const StoredFile &) = delete;
  StoredFile(StoredFile &&) = default;

  StoredFile &operator=(const StoredFile &) = delete;
  StoredFile &operator=(StoredFile &&) = default;

  ~StoredFile() override = default;

  void open() override;
  void close() override;
  void destroy() override;
  bool exists() const override;

  std::iostream *stream() override;

  std::pair<std::uintmax_t, std::error_code> size() const override;

  const std::filesystem::path &path() const;
  bool is_open() const;
};

// concept StoredObjectType = std::is_base_of_v<StoredObject, Object>;

/**
 * @brief Exclusive access to each object.
 */
class StoredObjectCollection {
public:
  using object_id_t = uint64_t;

  StoredObjectCollection() = default;
  virtual ~StoredObjectCollection() = default;

  virtual bool has(object_id_t id) const = 0;
  virtual object_id_t add() = 0;
  virtual std::iostream *get(object_id_t id) = 0;
  virtual std::iostream const &get(object_id_t id) const = 0;
  virtual std::pair<std::uintmax_t, std::error_code>
  size(object_id_t id) const = 0;
  virtual void destroy(object_id_t id) = 0;
  virtual void clear() = 0;
};

class StoredFolder : public StoredObjectCollection {
public:
  using object_id_t = StoredObjectCollection::object_id_t;

private:
  std::pmr::memory_resource *m_resource;
  std::filesystem::path m_root_path;
  std::pmr::unordered_map<object_id_t, UniquePtr<StoredFile>> m_files;
  std::atomic<object_id_t> m_next_object_id{};

public:
  using iterator_t =
      std::pmr::unordered_map<object_id_t, UniquePtr<StoredFile>>::iterator;
  using const_iterator_t =
      std::pmr::unordered_map<object_id_t,
                              UniquePtr<StoredFile>>::const_iterator;

  StoredFolder(std::pmr::memory_resource *resource,
               std::filesystem::path root_path,
               bool add_all_existing_files = true);

  StoredFolder(const StoredFolder &) = delete;
  StoredFolder(StoredFolder &&) = delete;
  StoredFolder &operator=(const StoredFolder &) = delete;
  StoredFolder &operator=(StoredFolder &&) = delete;

  ~StoredFolder() override = default;

  bool has(object_id_t id) const override;
  object_id_t add() override;
  std::iostream *get(object_id_t id) override;
  std::iostream const &get(object_id_t id) const override;
  std::pair<std::uintmax_t, std::error_code>
  size(object_id_t id) const override;
  void destroy(object_id_t id) override;
  void clear() override;

  std::filesystem::path const &path() const { return m_root_path; }

  iterator_t begin() { return m_files.begin(); }
  iterator_t end() { return m_files.end(); }
  const_iterator_t begin() const { return m_files.begin(); }
  const_iterator_t end() const { return m_files.end(); }
};

} // namespace objectstore
