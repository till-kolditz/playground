#pragma once

#include <cstdio>
#include <fcntl.h>
#include <filesystem>
#include <ios>
#include <ostream>
#include <stdio.h>
#include <streambuf>
#include <string>
#include <unistd.h>

/**
 * Adapted from https://stackoverflow.com/questions/77633473
 */
class stream_redirect {
public:
  [[gnu::nonnull]]
  stream_redirect(std::ostream *old_stream, std::ostream *new_stream)
      : m_old_stream{old_stream}, m_old_streambuf{old_stream->rdbuf()},
        m_new_stream{new_stream} {
    old_stream->rdbuf(new_stream->rdbuf());
  }

  ~stream_redirect() { m_old_stream->rdbuf(m_old_streambuf); }

private:
  std::ostream *m_old_stream;
  std::streambuf *m_old_streambuf;
  std::ostream *m_new_stream;
};

/**
 * Adapted from https://stackoverflow.com/questions/1908687
 */
class file_redirect {
  struct mode {
    bool read : 1;
    bool write : 1;
    bool append : 1;
    bool at_end : 1;
    bool truncate : 1;
    // ignoring ios::binary for now
  };

public:
  [[gnu::nonnull]]
  file_redirect(FILE *old_file, std::filesystem::path new_file_path,
                std::ios_base::openmode mode)
      : m_old_file{old_file}, m_old_file_fd{dup(fileno(m_old_file))},
        m_old_file_mode{fcntl64(m_old_file_fd, F_GETFL)},
        m_new_file_path{new_file_path} {
    parse_mode(mode);
    auto *modes = make_new_mode_string();
    m_new_file = std::freopen(m_new_file_path.c_str(), modes, m_old_file);
  }

  ~file_redirect() {
    fclose(m_new_file);

    const char *modes = make_old_mode_string();
    fclose(m_old_file);
    FILE *orig_file = fdopen(m_old_file_fd, modes);
    if (move_old_to_end()) {
      std::fseek(orig_file, 0, SEEK_END);
    }
    *m_old_file = *orig_file;
  }

  void flush() {
    if (m_new_file != nullptr) {
      fflush(m_new_file);
    }
  }

private:
  FILE *m_old_file{};
  int m_old_file_fd{};
  int m_old_file_mode{};
  std::filesystem::path m_new_file_path;
  FILE *m_new_file{};
  mode m_new_file_mode{};

  void parse_mode(std::ios_base::openmode mode) noexcept {
    m_new_file_mode.read = (mode & std::ios::in) == std::ios::in;
    m_new_file_mode.write = (mode & std::ios::out) == std::ios::out;
    m_new_file_mode.append = (mode & std::ios::app) == std::ios::app;
    m_new_file_mode.at_end = (mode & std::ios::ate) == std::ios::ate;
    m_new_file_mode.truncate = (mode & std::ios::trunc) == std::ios::trunc;
  }

  /**
   * Best-effort translation from std::ios_base::openmode to C fopen mode
   * string.
   */
  const char *make_new_mode_string() const noexcept {
    auto mode_str = std::string{};
    if (m_new_file_mode.read && m_new_file_mode.write) {
      if (m_new_file_mode.truncate) {
        return "w+";
      } else if (m_new_file_mode.append) {
        return "a+";
      } else {
        return "r+";
      }
    } else if (m_new_file_mode.write) {
      if (m_new_file_mode.truncate) {
        return "w+";
      } else if (m_new_file_mode.append) {
        return "a+";
      } else {
        return "w";
      }
    } else if (m_new_file_mode.truncate) {
      return "w+";
    } else if (m_new_file_mode.read) {
      return "r";
    }
    return "";
  }

  /**
   * Best-effort translation from saved old file mode to C fopen mode string.
   */
  const char *make_old_mode_string() const noexcept {
    if ((m_old_file_mode & O_RDWR) == O_RDWR) {
      if ((m_old_file_mode & O_APPEND) == O_APPEND) {
        return "a+";
      } else {
        return "r+";
      }
    } else if ((m_old_file_mode & O_WRONLY) == O_WRONLY) {
      return "a";
    } else {
      return "r";
    }
  }

  bool move_old_to_end() const noexcept {
    return (m_old_file_mode & O_APPEND) == O_APPEND;
  }
};
