#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <system_error>
#include <unistd.h>
#include <utility>

#include "objectstore.hpp"

namespace objectstore {

namespace {
std::filesystem::path
BuildPathForObjectStore(StoredFolder::object_id_t object_id,
                        std::filesystem::path const &root) {
  std::filesystem::path filepath = root;
  filepath += std::filesystem::path::preferred_separator;
  filepath += std::to_string(object_id);
  return filepath;
}

void EnsureFileStreamOpened(std::fstream *stream,
                            std::filesystem::path const &file_path) {
  if (!stream->is_open()) {
    stream->clear();
    stream->open(file_path, std::fstream::in | std::fstream::out);
    if (!stream->is_open()) {
      {
        std::ofstream{file_path};
      }
      stream->open(file_path, std::fstream::in | std::fstream::out);
    }
    assert(stream->is_open());
  }
}

} // namespace

void StoredFile::open() {
  EnsureFileStreamOpened(&m_stream, m_file_path);
  m_stream.seekg(0, std::ios::beg);
  assert(m_stream.tellg() == 0);
  m_stream.seekp(0, std::ios::beg);
  assert(m_stream.tellp() == 0);
  m_stream.flush();
  assert(m_stream.is_open());
}

void StoredFile::close() {
  m_stream.close();
  assert(!m_stream.is_open());
  m_stream.clear();
  assert(m_stream.good());
}

void StoredFile::destroy() {
  close();
  if (std::filesystem::exists(m_file_path)) {
    int res [[maybe_unused]] = unlink(m_file_path.c_str());
    assert(res == 0);
  }
}

bool StoredFile::exists() const { return std::filesystem::exists(m_file_path); }

std::iostream *StoredFile::stream() {
  open();
  return &m_stream;
}

std::pair<std::uintmax_t, std::error_code> StoredFile::size() const {
  auto ec = std::error_code{};
  auto size = std::filesystem::file_size(m_file_path);
  return std::make_pair(size, ec);
}

const std::filesystem::path &StoredFile::path() const { return m_file_path; }

bool StoredFile::is_open() const { return m_stream.is_open(); }

StoredFolder::StoredFolder(std::pmr::memory_resource *resource,
                           std::filesystem::path root_path,
                           bool add_all_existing_files)
    : m_resource{resource}, m_root_path{std::move(root_path)},
      m_files{resource} {
  if (!std::filesystem::exists(m_root_path)) {
    std::filesystem::create_directories(m_root_path);
  } else if (add_all_existing_files) {
    for (const auto &entry : std::filesystem::directory_iterator(m_root_path)) {
      if (entry.is_regular_file()) {
        auto filename = entry.path().filename().string();
        try {
          auto object_id =
              static_cast<StoredFolder::object_id_t>(std::stoull(filename));
          m_files.try_emplace(
              object_id, common::MakeUnique<StoredFile>(m_resource, m_resource,
                                                        entry.path()));
          if (object_id >= m_next_object_id) {
            m_next_object_id = object_id + 1;
          }
        } catch (const std::invalid_argument &) {
        } catch (const std::out_of_range &) {
        }
      }
    }
  }
}

bool StoredFolder::has(object_id_t id) const { return m_files.contains(id); }

StoredFolder::object_id_t StoredFolder::add() {
  auto id = m_next_object_id++;
  auto file_path = BuildPathForObjectStore(id, m_root_path);
  m_files.try_emplace(
      id, common::MakeUnique<StoredFile>(m_resource, m_resource, file_path));
  return id;
}

std::iostream *StoredFolder::get(object_id_t id) {
  auto it = m_files.find(id);
  if (it == m_files.end()) {
    return nullptr;
  }
  auto &file = it->second;
  file->open();
  return file->stream();
}

std::iostream const &StoredFolder::get(object_id_t id) const {
  auto it = m_files.find(id);
  if (it == m_files.end()) {
    throw std::out_of_range{"Object ID not found in StoredFolder"};
  }
  auto &file = it->second;
  file->open();
  return *(file->stream());
}

std::pair<std::uintmax_t, std::error_code>
StoredFolder::size(object_id_t id) const {
  auto it = m_files.find(id);
  if (it == m_files.end()) {
    return std::make_pair(
        0, std::make_error_code(std::errc::no_such_file_or_directory));
  }
  auto &file = it->second;
  return file->size();
}

void StoredFolder::destroy(object_id_t id) {
  if (auto it = m_files.find(id); it != m_files.end()) {
    auto &file = it->second;
    file->destroy();
    m_files.erase(it);
  }
}

void StoredFolder::clear() {
  for (auto const &pair : m_files) {
    pair.second->destroy();
  }
  m_files.clear();
}

} // namespace objectstore
