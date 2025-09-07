#include <signal.h>
#include <iostream>
#include <thread>
#include <chrono>

int gInt = 3;

void signal_handler(int sig)
{
  gInt = 4;
  for (uint i = 0; i < 5; ++i)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "I'm signal handling thread: " << std::this_thread::get_id() << ", couting: " << i << std::endl;
  }
  std::cout << "I'm signla hadnling thread, global int: " << gInt << std::endl;
}

int main()
{
  signal(SIGINT, signal_handler);
  for (uint i = 0; i < 8; ++i)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "I'm main thread: " << std::this_thread::get_id() << ", couting: " << i << std::endl;
  }
  std::cout << "I'm main thread, global int: " << gInt << std::endl;
  return 0;
}