// Shared memory demo - common definitions
#pragma once

#include <cstddef>

// Named resources (start with '/' as required by POSIX)
inline constexpr const char *SHM_NAME = "/shm_demo_v1";
inline constexpr const char *SEM_EMPTY_NAME = "/shm_demo_empty_v1"; // Semaphores 信号量
inline constexpr const char *SEM_FULL_NAME = "/shm_demo_full_v1";

inline constexpr std::size_t SHM_BUFFER_SIZE = 256;

struct SharedData
{
  std::size_t len; // number of valid bytes in buffer
  char buffer[SHM_BUFFER_SIZE];
};
