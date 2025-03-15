#include "../include/helper.hh"

int main()
{
	bool runAll = true;
	if (false || runAll) // Bit operation
	{
		// 位异或
		INFO_STREAM("0b011 ^ 0b101: " << (0b011 ^ 0b101)); // 6
		INFO_STREAM("0b011 | 0b101: " << (0b011 | 0b101));// 7
		INFO_STREAM("0b011 & 0b101: " << (0b011 & 0b101)); // 1
	}
	if (false || runAll) // Bit operation
	{
		char mode_data[4] = {0x30, 0x00, 0x00, 0x00};
		std::cout << mode_data << std::endl;
		mode_data[0] = 0x31;
		mode_data[1] = 0x12;
		mode_data[2] = (mode_data[2] & 0x0F) | 0x20;
		std::string s;
		s += mode_data;
		std::cout << s << std::endl;
	}
	if (false || runAll) // bit operation
	{
		uint32_t number = 0xABCD1234; // 示例32位整数

		// 提取低4位
		uint32_t low4Bits = number & 0xF; // 0xF 是二进制的 00001111
		std::cout << "低4位: " << low4Bits << std::endl;

		// 提取高28位
		uint32_t high28Bits = number >> 4; // 右移4位后，低4位被丢弃
		high28Bits &= 0xFFFFFFF;					 // 0xFFFFFFF 是二进制的 1111111111111111111111111111
		std::cout << "高28位: " << high28Bits << std::endl;
	}
}