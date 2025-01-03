#include <chrono>
#include <thread>
#include "../include/helper.hh"

int main()
{
    auto start = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds> (std::chrono::steady_clock::now()-start);
    INFO_STREAM("Duration: " << duration.count());
}