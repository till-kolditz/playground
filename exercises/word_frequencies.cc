#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using Frequencies = std::unordered_map<std::string, size_t>;

/**
 * Sentences are tokenized using space as separator.
 *
 * @return count of words across all sentences (frequencies)
 */
Frequencies word_counts(std::vector<std::string> &sentences) {
  auto result = std::unordered_map<std::string, size_t>{};

  const char *delim = " ";
  for (std::string &sentence : sentences) { // O(N)
    if (sentence.empty()) {
      continue;
    }
    auto *buf = sentence.data();
    char *token = std::strtok(buf, delim);
    while (token != nullptr) {                                    // O(M)
      if (auto iter = result.find(token); iter != result.end()) { // O(1)
        ++iter->second;
      } else {
        result.emplace(token, 1); // O(1)
      }
      token = std::strtok(nullptr, delim);
    }
  }

  return result;
}

void compare(Frequencies const &result, Frequencies const &gold_std) {
  if (result.size() != gold_std.size()) {
    auto ss = std::stringstream{};
    ss << "result.size(" << result.size() << ") != gold_std.size("
       << gold_std.size() << ')';
    bool first_entry = true;
    for (auto const &result_entry : result) {
      if (!gold_std.contains(result_entry.first)) {
        if (first_entry) {
          ss << "\nSuperfluous words: ";
          first_entry = false;
        } else {
          ss << ", ";
        }
        ss << result_entry.first;
      }
    }
    for (auto const &gold_std_entry : gold_std) {
      if (!result.contains(gold_std_entry.first)) {
        if (first_entry) {
          ss << "\nMissing words: ";
          first_entry = false;
        } else {
          ss << ", ";
        }
        ss << gold_std_entry.first;
      }
    }
    throw std::runtime_error{ss.str()};
  }
  for (auto const &result_entry : result) {
    auto gold_std_iter = gold_std.find(result_entry.first);
    if (gold_std_iter == gold_std.end()) {
      auto ss = std::stringstream{};
      ss << "unexpected \"" << result_entry.first << "\" found";
      throw std::runtime_error{ss.str()};
    }
    if (result_entry.second != gold_std_iter->second) {
      auto ss = std::stringstream{};
      ss << "expected \"" << result_entry.first << "\" to appear "
         << gold_std_iter->second << " times, but got " << result_entry.second
         << " times";
      throw std::runtime_error{ss.str()};
    }
  }
  std::cout << "Input matches expectation.\n";
}

int main() {
  auto test_case = std::vector<std::string>{
      "this is my first sentence and my first test",
      "here is another sentence let us see if this is my third thing"};
  auto frequencies = word_counts(test_case);
  for (auto const &entry : frequencies) {
    std::cout << entry.first << " : " << entry.second << '\n';
  }
  auto gold_std = std::unordered_map<std::string, size_t>{
      {"thing", 1}, {"this", 2},    {"is", 3},   {"third", 1},    {"let", 1},
      {"my", 3},    {"another", 1}, {"see", 1},  {"sentence", 2}, {"and", 1},
      {"first", 2}, {"test", 1},    {"here", 1}, {"if", 1},       {"us", 1}};
  compare(frequencies, gold_std);
}
