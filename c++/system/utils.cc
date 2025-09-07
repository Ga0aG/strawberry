#include <iostream>
#include <sys/times.h>
#include <unistd.h>
#include <iomanip>
#include <fstream>

int main()
{
	{ // 计算程序在内核态和用户态的时间

		struct tms t;
		clock_t start, end;

		// 获取初始时间
		start = times(&t);

		// 这里可以放置你想要测量时间的代码
		for (volatile long i = 0; i < 100000000; ++i)
			;

		std::ifstream fin;
		fin.open("./utils.cc");

		// // 获取结束时间
		// end = times(&t);

		// // 计算用户态和内核态时间
		// double user_time = (double)(t.tms_utime) / sysconf(_SC_CLK_TCK);
		// double system_time = (double)(t.tms_stime) / sysconf(_SC_CLK_TCK);

		// std::cout << "用户态时间: " << std::fixed << std::setprecision(10) << user_time << " 秒" << std::endl;
		// std::cout << "内核态时间: " << std::fixed << std::setprecision(10) << system_time << " 秒" << std::endl;
	}

	return 0;
}