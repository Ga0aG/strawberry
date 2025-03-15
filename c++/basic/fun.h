#ifndef __TEST_FUNCTION__
#define __TEST_FUNCTION__
static int staticVar = 1;
void IncreaseStaticVar1();
void IncreaseStaticVar2();

// function included in multiple source files must be inline
// 不然如果被多个源文件引用，会有错误: `multiple definition of `sum(int, int)'`
inline int sum(int a, int b)
{
	return a+b;
};

#endif