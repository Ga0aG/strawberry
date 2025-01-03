#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <thread>
#include <map>

int main()
{
    char mode_data[4] = {0x30, 0x00, 0x00, 0x00};
    std::cout << mode_data << std::endl;
    //
    mode_data[0] = 0x31;
    mode_data[1] = 0x12;
    mode_data[2] = (mode_data[2] & 0x0F) | 0x20;
    std::string s;
    s += mode_data;
    std::cout << s << std::endl;
    {// float
        std::cout << (1e20 + -1e20) + 3.14 << ", " << 1e20 + (-1e20 + 3.14) << std::endl;
    }
    {
        char num_buf[4] = {0};
        sprintf(num_buf, "%03u", 999);
        std::string msg = "\x02";
        // msg += num_buf;
        // sprintf(num_buf, "%03u", 3);
        // msg += num_buf;
        // std::cout << msg << std::endl;
        std::cout << std::setfill('0') << std::setw(2) << std::hex << (uint32_t)(uint8_t)(msg[0]) << std::endl;

        std::stringstream oss;
        char a = '0';
        oss << std::setfill('0') << std::setw(2) << std::hex << (uint32_t)(uint8_t)a;
        std::cout << oss.str() << std::endl;
        oss.str("");
        oss << std::setfill('0') << std::setw(2) << std::hex << (uint32_t)(uint8_t)0;
        std::cout << oss.str() << std::endl;
        oss.str("");
        oss << std::setw(3) << std::setfill('0') << 999;
        std::cout << oss.str() << std::endl;

        std::chrono::time_point<std::chrono::steady_clock> now1 = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::microseconds(100000));
        std::chrono::time_point<std::chrono::steady_clock> now2 = std::chrono::steady_clock::now();
        std::chrono::duration<double> waitingResponseTime =  std::chrono::duration_cast<std::chrono::microseconds> (now2 - now1);
        std::cout << waitingResponseTime.count() << std::endl;
    }
    {// bit operation
        uint32_t number = 0xABCD1234;  // 示例32位整数

        // 提取低4位
        uint32_t low4Bits = number & 0xF; // 0xF 是二进制的 00001111
        std::cout << "低4位: " << low4Bits << std::endl;

        // 提取高28位
        uint32_t high28Bits = number >> 4; // 右移4位后，低4位被丢弃
        high28Bits &= 0xFFFFFFF; // 0xFFFFFFF 是二进制的 1111111111111111111111111111
        std::cout << "高28位: " << high28Bits << std::endl;
    }
    {// static cast
        enum LightModuleState : uint8_t
        {
            LMS_UNINITIALIZED = 0,
            LMS_INITIALIZED = 1,
            LMS_OPERATION = 2,
            LMS_FINISHED = 3,
            LMS_ERROR = 4
        };

        uint32_t stateAndDisplay = 0xFFF; // Example value
        LightModuleState state = static_cast<LightModuleState>(stateAndDisplay & 0xFF);
        std::cout << "State: " << static_cast<int>(state) << std::endl;
        uint32_t highState = (stateAndDisplay>>8)&0xFFFFFF;
        std::cout << "State: " << static_cast<int>(highState) << std::endl;
    }
    {
        std::string a;
        std::cout << (std::string() == a) << std::endl;
    }
    return 0;
}
