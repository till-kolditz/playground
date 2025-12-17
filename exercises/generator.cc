#include <iostream>

#include "generator.hpp"

int main() {
  for (auto num : NumberRange{10}) {
    std::cout << num << '\n';
  }
}
