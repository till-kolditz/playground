#include <array>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>

#include "stream_redirect.hpp"

using std::string_view_literals::operator""sv;

int read_file_stream(std::filesystem::path const &path, std::fstream *fs) {
  auto &stream = *fs;
  stream.flush();
  if (!stream) {
    std::cout << "Error flushing file: " << std::strerror(errno) << "\n";
    return errno;
  }
  stream.sync();
  if (!stream) {
    std::cout << "Error syncing file: " << std::strerror(errno) << "\n";
    return errno;
  }
  stream.seekg(0, std::ios::beg);
  if (!stream) {
    std::cout << "Error seeking to file start: " << std::strerror(errno)
              << "\n";
    return errno;
  }

  size_t file_size = std::filesystem::file_size(path);
  std::cout << "file size: " << file_size << '\n';
  auto buf = std::array<char, 1024>{};
  size_t total_gcount = 0;
  while (stream && total_gcount < buf.size()) {
    size_t read_size = buf.size() - total_gcount;
    stream.read(buf.data() + total_gcount, read_size);
    size_t gcount = stream.gcount();
    std::cout << "read count: " << gcount << '\n';
    total_gcount += gcount;
  }
  if (!stream && !stream.eof()) {
    std::cout << "Error reading from file: " << std::strerror(errno)
              << " fail=" << (stream.fail() ? "yes" : "no")
              << " bad=" << (stream.bad() ? "yes" : "no") << "\n";
    return errno;
  }
  auto view = std::string_view{buf.data(), total_gcount};
  std::cout << "File contents:\n=====\n" << view << "\n=====\n";
  return 0;
}

int main(int argc, char *argv[]) {
  {
    std::cout << "Redirecting std::cerr to internal stringstream.\n";
    auto ss = std::stringstream{};
    auto redirect = stream_redirect{&std::cerr, &ss};
    std::cerr << "test error message1"sv;
    std::cout << "redirect received '" << ss.str() << "'\n";
  }
  {
    auto path = std::filesystem::path{argv[0]};
    path += ".err1";
    std::cout << "Redirecting std::cerr to '" << path << "'.\n";
    auto fs = std::fstream{path, std::ios::in | std::ios::out | std::ios::app |
                                     std::ios::ate};
    if (!fs) {
      std::cout << "Error opening file: " << std::strerror(errno) << "\n";
      return errno;
    }
    auto redirect = stream_redirect{&std::cerr, &fs};
    std::cerr << "test error message2\n"sv;
    int err = read_file_stream(path, &fs);
    if (err != 0) {
      return err;
    }
  }

  std::cerr << "This should go to the default std::cerr stream again!\n";

  {
    auto path = std::filesystem::path{argv[0]};
    path += ".err2";
    std::cout << "Redirecting stderr to '" << path << "'.\n";

    auto fr = file_redirect{stderr, path, std::ios::out | std::ios::trunc};
    fprintf(stderr, "test error message3\n");
    fr.flush();

    auto fs = std::fstream{path, std::ios::in | std::ios::out | std::ios::app |
                                     std::ios::ate};
    if (!fs) {
      std::cout << "Error opening file: " << std::strerror(errno) << "\n";
      return errno;
    }
    int err = read_file_stream(path, &fs);
    if (err != 0) {
      return err;
    }
  }

  fprintf(stderr, "This should go to the default stderr stream again!\n");
}
