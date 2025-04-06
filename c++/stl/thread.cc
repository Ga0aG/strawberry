#include "../include/helper.hh"
#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>

std::atomic<bool> ready(false);

void count1m(int id) {
  while (!ready) { // wait until main() sets ready...
    std::this_thread::yield();
  }
  for (volatile int i = 0; i < 1000000; ++i) {
  }
  std::cout << id;
}

std::mutex mtx; // mutex for critical section
void print_block(int n, char c) {
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  mtx.lock();
  std::cout << now();
  for (int i = 0; i < n; ++i) {
    std::cout << c;
  }
  std::cout << '\n';
  mtx.unlock();
}

void worker(std::promise<int> p_notice) {
  std::cout << now() << "Preparing..." << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  p_notice.set_value(1);
}

void customer(std::shared_future<int> f_notice) {
  mtx.lock();
  int notice = f_notice.get();
  std::cout << now() << std::this_thread::get_id() << " recived response"
            << std::endl;
  mtx.unlock();
}

int main()
{
  bool runAll = true;
  if(false || runAll) // spawn thread
  {
    print_header("Test thread");
    std::thread threads[10];
    std::atomic<bool> ready1(false);
    for (int i = 0; i < 10; ++i) {
      threads[i] = std::thread(
          [&ready1](int id) {
            while (!ready1) { // wait until main() sets ready1...
              std::this_thread::yield();
            }
            for (volatile int i = 0; i < 1000000; ++i) {
            }
            std::cout << id;
          },
          i);
    }
    ready1 = true; // go!
    for (auto &th : threads)
      th.join();
    std::cout << '\n';
  }
  if(false || runAll) // async & launch policy
  {
    print_header("Test async & launch policy");
    std::cout << now() << "Test async policy" << std::endl;
    auto future = std::async(std::launch::async, print_block, 1,
                             'H'); // "H" is const char [2]
    auto print_future_status = [&future]() {
      std::future_status status;
      switch (status = future.wait_for(std::chrono::milliseconds(100))) {
      case std::future_status::deferred:
        std::cout << now() << "deferred status" << std::endl;
        break;
      case std::future_status::timeout:
        std::cout << now() << "timeout status" << std::endl;
        break;
      case std::future_status::ready:
        std::cout << now() << "ready status" << std::endl;
        break;
      }
    };
    print_future_status();
    while (!future.valid()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    future.wait();
    print_future_status(); // ready

    std::cout << now() << "Test deferred policy" << std::endl;
    future = std::async(std::launch::deferred, print_block, 1, 'H');
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    print_future_status(); // deferred
    future.get();          // function was called in wait/get
    // print_future_status(); // No associated state
  }
  if(false || runAll) // mutex
  {
    print_header("Test mutex");
    // std::thread th1 (print_block,500,'*');
    // std::thread th2 (print_block,500,'$');
    // th1.join();
    // th2.join();
  }
  if(false || runAll) // shared_future & promise
  {
    print_header("Test shared_future");
    std::promise<int> p_notice;
    std::shared_future<int> f_notice1 = p_notice.get_future().share();
    std::shared_future<int> f_notice2 = f_notice1;
    std::thread customer1(customer, std::move(f_notice1));
    std::thread customer2(customer, std::move(f_notice2));
    std::thread worker1(worker, std::move(p_notice));
    customer1.join();
    customer2.join();
    worker1.join();
  }
  if(false || runAll) // condition variable
  {
    print_header("Test condition variable");
    std::mutex mtx;
    std::condition_variable cv;
    auto fun = [&mtx, &cv](int id) {
      std::unique_lock<std::mutex> lck(mtx);
      cv.wait(lck);
      std::cout << now() << id << std::endl;
    };
    auto go = [&mtx, &cv]() {
      // chatgpt
      // The reason for acquiring a lock (std::unique_lock<std::mutex>
      // lck(mtx);) before calling cv.notify_all() is to ensure that the
      // modification of the shared state (in this case, the condition variable)
      // is done atomically and is protected by the mutex. This is a common
      // pattern in multithreaded programming to prevent race conditions.
      std::unique_lock<std::mutex> lck(mtx);
      std::cout << now() << "Run threads" << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      cv.notify_all();
    };
    std::thread threads[10];
    for (unsigned int i = 0; i < 10; ++i) {
      threads[i] = std::thread(fun, i);
    }
    std::cout << now() << "Spawn 10 threads" << std::endl;
    std::thread alarm_thread = std::thread(go);
    for (auto &thread : threads) {
      thread.join();
    }
    alarm_thread.join();
  }
  if(false || runAll) // atomic
  {
    print_header("Test atomic");
    std::atomic_int num{0};
    int normal_int{0};
    auto fun = [&num, &normal_int](int n) {
      for (int i = 0; i < n; ++i) {
        ++num;
        ++normal_int; // Add lock to it will increase execution time
      }
    };
    std::thread threads[10];
    for (unsigned int i = 0; i < 10; ++i) {
      threads[i] = std::thread(fun, i * 10 + 1);
    }
    for (auto &thread : threads) {
      thread.join();
    }
    std::cout << now() << "Atomic int Sum:" << num.load() << std::endl;
    std::cout << now() << "Normal int Sum:" << normal_int << std::endl;
  }
  if(false || runAll) // thread pool
  {
    // #include "../3rd/thread-pool/include/ThreadPool.h"
    // https://github.com/mtrebi/thread-pool/tree/master
    /**
		 * @brief Why thread pool?
		 * 线程过多或者频繁创建和销毁线程会带来调度开销，进而影响缓存局部性和整体性能。而线程池维护着多个线程，等待着管理器分配可并发执行的任务。这**避免了在处理短时间任务时创建与销毁线程的代价**，以及保证了线程的可复用性。
		 * csapp-ch12.3.1 线程切换和进程切换很相似，也需要上下文切换。
		 * csapp-ch8.2.5 进程上下文切换:
		 * (1) saves the context of the current process,
		 * (2) restores the saved context of some previously preempted挂起的 process, and
		 * (3) passes control to this newly restored process.
		 * csapp-ch8.2.4 内核态与用户态
		 * 处理器使用某个控制寄存器中的一个模式位（mode bit）来区分用户模式与内核模式。
		 * 运行在内核模式的进程可以执行指令集中的任何指令，并可以访问系统中的任何内存位置。
		 * 运行在用户模式的进程不允许执行特权指令，比如停止处理器、改变模式位、发起 I/O 操作等，也不能直接引用地址空间内核区中的代码和数据，用户程序只能通过系统调用接口间接地访问内核代码和数据。进程从用户模式变为内核模式的方法是通过中断、故障、陷阱（系统调用就是陷阱）这样的异常。
		 */
  }
  return 0;
}
