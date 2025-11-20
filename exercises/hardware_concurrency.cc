#include <iostream>
#include <thread>

int main() {
  std::cout << std::jthread::hardware_concurrency()
            << " concurrent threads are supported.\n";
}
