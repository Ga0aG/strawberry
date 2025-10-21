#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>

#include <semaphore.h>

#include "shm_common.h"

int main()
{
  // Open existing shared memory and semaphores (writer creates/unlinks them)
  int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
  if (shm_fd == -1)
  {
    std::perror("shm_open");
    return 1;
  }

  SharedData *shared = static_cast<SharedData *>(mmap(nullptr, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
  if (shared == MAP_FAILED)
  {
    std::perror("mmap");
    close(shm_fd);
    return 1;
  }

  sem_t *sem_empty = sem_open(SEM_EMPTY_NAME, 0);
  if (sem_empty == SEM_FAILED)
  {
    std::perror("sem_open empty");
    munmap(shared, sizeof(SharedData));
    close(shm_fd);
    return 1;
  }
  sem_t *sem_full = sem_open(SEM_FULL_NAME, 0);
  if (sem_full == SEM_FAILED)
  {
    std::perror("sem_open full");
    sem_close(sem_empty);
    munmap(shared, sizeof(SharedData));
    close(shm_fd);
    return 1;
  }

  while (true)
  {
    if (sem_wait(sem_full) == -1)
    {
      std::perror("sem_wait full");
      break;
    }

    std::size_t n = shared->len;
    if (n > SHM_BUFFER_SIZE)
      n = SHM_BUFFER_SIZE;
    std::string msg(shared->buffer, shared->buffer + n);

    std::fprintf(stdout, "reader: got %zu bytes: '%s'\n", n, msg.c_str());

    if (sem_post(sem_empty) == -1)
    {
      std::perror("sem_post empty");
      break;
    }

    if (msg == "END")
    {
      break;
    }
  }

  sem_close(sem_empty);
  sem_close(sem_full);
  munmap(shared, sizeof(SharedData));
  close(shm_fd);
  // Do not unlink here; writer handles unlink so resources disappear when both close
  return 0;
}
