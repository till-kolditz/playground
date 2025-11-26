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

int main(int argc, char *argv[]) {
  {
    auto ss = std::stringstream{};
    auto redirect = stream_redirect{&std::cerr, &ss};
    std::cerr << "test error message1"sv;
    std::cout << "redirect received '" << ss.str() << "'\n";
  }
  {
    auto path = std::filesystem::path{argv[0]};
    path += ".out";
    std::cout << "Going to redirect std::cerr to '" << path << "'\n";
    auto fs = std::fstream{path, std::ios::in | std::ios::out | std::ios::app |
                                     std::ios::ate};
    if (!fs) {
      std::cout << "Error opening file: " << std::strerror(errno) << "\n";
      return errno;
    }
    auto redirect = stream_redirect{&std::cerr, &fs};
    std::cerr << "test error message2\n"sv;
    fs.flush();
    if (!fs) {
      std::cout << "Error flushing file: " << std::strerror(errno) << "\n";
      return errno;
    }
    fs.sync();
    if (!fs) {
      std::cout << "Error syncing file: " << std::strerror(errno) << "\n";
      return errno;
    }
    fs.seekg(0, std::ios::beg);
    if (!fs) {
      std::cout << "Error seeking to file start: " << std::strerror(errno)
                << "\n";
      return errno;
    }
    size_t file_size = std::filesystem::file_size(path);
    std::cout << "file size: " << file_size << '\n';
    auto buf = std::array<char, 1024>{};
    size_t total_gcount = 0;
    while (fs && total_gcount < buf.size()) {
      size_t read_size = buf.size() - total_gcount;
      fs.read(buf.data() + total_gcount, read_size);
      size_t gcount = fs.gcount();
      std::cout << "read count: " << gcount << '\n';
      total_gcount += gcount;
    }
    if (!fs && !fs.eof()) {
      std::cout << "Error reading from file: " << std::strerror(errno)
                << " fail=" << (fs.fail() ? "yes" : "no")
                << " bad=" << (fs.bad() ? "yes" : "no") << "\n";
      return errno;
    }
    auto view = std::string_view{buf.data(), total_gcount};
    std::cout << "File contents:\n=====\n" << view << "\n=====\n";
  }

  std::cerr << "This should go to the default std::cerr stream again!\n";
}
