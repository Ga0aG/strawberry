#pragma once

#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// auto start = std::chrono::system_clock::now();
inline std::string now() {
  // auto end = std::chrono::system_clock::now();
  // std::chrono::duration<double> elapsed_seconds = end - start;
  auto end = std::chrono::system_clock::now();
  auto duration = end.time_since_epoch();
  double seconds =
      std::chrono::duration_cast<std::chrono::microseconds>(duration).count() *
      1e-6;
  return "[" + std::to_string(seconds) + "] ";
};

#define INFO_STREAM(stream_arg)                                                \
  do {                                                                         \
    std::stringstream ss;                                                      \
    ss << stream_arg;                                                          \
    std::cout << now() << ss.str() << std::endl;                               \
  } while (0)

inline void print_header(const std::string &header) {
  std::cout << std::fixed << std::setprecision(6);
  std::cout << now() << "***************************" << std::endl;
  std::cout << now() << "== " << header << " ==" << std::endl;
}

template <typename T> inline void print_vector(std::vector<T> vec) {
  std::cout << now();
  for (auto i : vec) {
    std::cout << i << ", ";
  }
  std::cout << std::endl;
};

template <typename T, std::size_t N>
inline void print_array(std::array<T, N> vec) {
  std::cout << now();
  for (auto i : vec) {
    std::cout << i << ", ";
  }
  std::cout << std::endl;
};

// ‘sizeof’ on array function parameter ‘arr’ will return size of ‘int*’
template <typename T> inline void print_array(T arr[], unsigned long int n) {
  std::cout << now();
  for (unsigned int ind = 0; ind < n; ++ind) {
    std::cout << arr[ind] << ", ";
  }
  std::cout << std::endl;
}

template <typename T1, typename T2>
inline void print_map(std::map<T1, T2> mapp) {
  std::cout << now() << "{";
  for (auto m : mapp) {
    std::cout << "{" << m.first << ", " << m.second << "}";
  }
  std::cout << "}";
  std::cout << std::endl;
};
