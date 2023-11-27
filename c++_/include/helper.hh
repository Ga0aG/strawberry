#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

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
}

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
