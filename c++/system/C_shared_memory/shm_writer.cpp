#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

#include <semaphore.h>

#include "shm_common.h"

namespace
{

  int shm_fd = -1;
  SharedData *shared_data = nullptr;
  sem_t *sem_empty = SEM_FAILED;
  sem_t *sem_full = SEM_FAILED;

  void cleanup()
  {
    if (shared_data && shared_data != MAP_FAILED)
    {
      munmap(shared_data, sizeof(SharedData));
      /**
       * int munmap(void *addr, size_t len);
       * Remove any mappings for those entire pages containing any part of the address space of the process starting at addr and continuing for len bytes.
       */
      shared_data = nullptr;
    }
    if (shm_fd != -1)
    {
      close(shm_fd);
      shm_fd = -1;
    }
    // Writer is responsible for unlinking named objects
    shm_unlink(SHM_NAME);
    if (sem_empty && sem_empty != SEM_FAILED)
    {
      sem_close(sem_empty);
      sem_empty = SEM_FAILED;
    }
    if (sem_full && sem_full != SEM_FAILED)
    {
      sem_close(sem_full);
      sem_full = SEM_FAILED;
    }
    sem_unlink(SEM_EMPTY_NAME);
    sem_unlink(SEM_FULL_NAME);
  }

  void on_signal(int)
  {
    cleanup();
    std::fprintf(stderr, "\nwriter: cleaned up and exiting on signal\n");
    std::_Exit(0);
  }

} // namespace

int main()
{
  std::signal(SIGINT, on_signal);
  std::signal(SIGTERM, on_signal);

  // I. 创建共享内存
  shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
  /**
   * int shm_open(const char *name, int oflag, mode_t mode);
   * @param name: 共享内存对象的唯一名称
   * @param oflag: 打开标志位
   * @param mode: 权限模式
   * vs open, 都是返回文件描述符，但是shm_open创建的对象位于/dev/shm中，也就是内存中
   */
  if (shm_fd == -1)
  {
    std::perror("shm_open");
    return 1;
  }

  // II. 分配内存大小
  if (ftruncate(shm_fd, static_cast<off_t>(sizeof(SharedData))) == -1)
  {
    /**
     * int ftruncate(int fildes, off_t length);
     * 调整打开文件的大小。特别是在处理大文件时, 其基本作用是将文件的大小截断到指定的值。新创建的共享内存大小为0，必须显式设置大小
     * @param fildes: The fildes argument is not a file descriptor open for writing.
     */
    std::perror("ftruncate");
    cleanup();
    return 1;
  }

  // III. 将文件映射进进程的虚拟内存空间
  shared_data = static_cast<SharedData *>(mmap(nullptr, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
  /**
   * void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
   * 内存映射文件。将一个文件或设备映射到进程的地址空间，使得可以通过指针直接访问文件的内容，而无需使用传统的文件读写操作（如 read 和 write）
   * @param prot: 保护权限
   * - PROT_READ: Page can be read
   * @param flags: 控制映射行为的选项
   * - MAP_SHARED： 修改对所有映射的进程可见（用于IPC）
   * - PRIVATE：修改只对当前进程可见（写时复制）
   */
  if (shared_data == MAP_FAILED)
  {
    std::perror("mmap");
    cleanup();
    return 1;
  }

  // IV. 创建信号量
  // Create semaphores: empty starts at 1 (buffer free), full at 0 (no data yet)
  // SEM_EMPTY 初始为 1：表示缓冲区空，写者可写。
  // SEM_FULL 初始为 0：表示暂无数据，读者需等待。
  sem_empty = sem_open(SEM_EMPTY_NAME, O_CREAT, 0666, 1);
  /**
   * sem_t *sem_open(const char *name, int oflag, ...,  mode_t mode, unsigned int value)
   * 打开信号量
   */
  if (sem_empty == SEM_FAILED)
  {
    std::perror("sem_open empty");
    cleanup();
    return 1;
  }
  sem_full = sem_open(SEM_FULL_NAME, O_CREAT, 0666, 0);
  if (sem_full == SEM_FAILED)
  {
    std::perror("sem_open full");
    cleanup();
    return 1;
  }

  std::vector<std::string> messages = {
      "hello from writer",
      "this is a POSIX shared memory demo",
      "synchronization via named semaphores",
      "message 3",
      "END" // sentinel to tell reader to stop
  };

  // V. 写一条数据，sem_empty等待reader读取，再写一条
  for (const std::string &msg : messages)
  {
    if (sem_wait(sem_empty) == -1)
    { // 如果为0则阻塞, 信号量减1
      std::perror("sem_wait empty");
      cleanup();
      return 1;
    }

    std::size_t n = msg.size();
    if (n > SHM_BUFFER_SIZE)
      n = SHM_BUFFER_SIZE;
    std::memcpy(shared_data->buffer, msg.data(), n);
    shared_data->len = n;

    // Make the data visible to other processes (usually not needed, but safe)
    msync(shared_data, sizeof(SharedData), MS_SYNC);
    /**
     * int msync(void *addr, size_t length, int flags);
     * 将内存映射区域的修改同步回底层存储（文件或共享内存）
     */

    if (sem_post(sem_full) == -1)
    { // 信号量加1，唤醒等待进程
      std::perror("sem_post full");
      cleanup();
      return 1;
    }

    std::fprintf(stdout, "writer: sent %zu bytes: '%.*s'\n", n, (int)n, shared_data->buffer);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }

  cleanup();
  return 0;
}
