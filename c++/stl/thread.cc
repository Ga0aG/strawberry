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

int main() {
  { // thread
    print_header("Test thread");
    std::thread threads[10];
    for (int i = 0; i < 10; ++i) {
      threads[i] = std::thread(
          [](int id) {
            while (!ready) { // wait until main() sets ready...
              std::this_thread::yield();
            }
            for (volatile int i = 0; i < 1000000; ++i) {
            }
            std::cout << id;
          },
          i);
    }
    ready = true; // go!
    for (auto &th : threads)
      th.join();
    std::cout << '\n';
  }

  { // async & launch policy
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

  { // mutex
    print_header("Test mutex");
    // std::thread th1 (print_block,500,'*');
    // std::thread th2 (print_block,500,'$');
    // th1.join();
    // th2.join();
  }

  { // shared_future & promise
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

  { // condition variable
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

  { // atomic
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

  return 0;
}
