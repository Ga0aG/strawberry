#include <chrono>
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include "../../include/helper.hh"

// g++ boost/asio/C_misc.cpp -o ./build/boost/misc -lpthread

int main()
{
    {// dealine timer
        print_header("Test deadline_timer");
        boost::asio::io_service io;
        boost::asio::deadline_timer connectTimer(io);
        connectTimer.expires_from_now(boost::posix_time::seconds(2));
        connectTimer.async_wait([](const boost::system::error_code& error){
            INFO_STREAM("Time out");
        });
        INFO_STREAM("Start to run one");
        io.run_one();
        INFO_STREAM("Called io run one");
        connectTimer.cancel();
        INFO_STREAM("Called timer cancel");
        std::this_thread::sleep_for(std::chrono::seconds(3));
        io.stop();
    }
    return 0;
}