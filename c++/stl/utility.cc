#include "../include/helper.hh"
#include <utility>
#include <functional>

template <typename T>
void print(T &t)
{
	INFO_STREAM(""
							<< "左值");
}
template <typename T>
void print(T &&t)
{
	INFO_STREAM(""
							<< "右值");
}
template <typename T>
void testForward(T &&v)
{
	INFO_STREAM(""
							<< ">>>>>>");
	print(v);
	print(std::forward<T>(v));
	print(std::move(v));
}

template <typename T>
void f(T &&param) {}

// a function:
int TestFunction(int x) { return x / 2; }
// a function object class:
struct TestFunctionStruct
{
	int operator()(int x) { return x / 3; }
};

double TestBind(double x, double y) { return x / y; }
struct TestBindStruct
{
	double a, b;
	double multiply() { return a * b; }
};

int main()
{
	bool runAll = true;
	// <utility>
	if (false || runAll) // universal references
	{
		// Rvalue references只能绑定到右值上，lvalue references除了可以绑定到左值上，在某些条件下还可以绑定到右值上。规则简化如下：
		// 左值引用   {左值}
		// 右值引用   {右值}
		// 常左值引用  {右值}
		// std::string &s = "asd";  // error
		const std::string &s = "asd"; // ok
		// > 一个universal reference必须具有形如T&&
		int a;
		f(a); // 传入左值,那么上述的T&& 就是lvalue reference,也就是左值引用绑定到了左值
		f(1); // 传入右值,那么上述的T&& 就是rvalue reference,也就是右值引用绑定到了左值
					// > 引用折叠只有两条规则:
					// 一个 rvalue reference to an rvalue reference 会变成 (“折叠为”) 一个 rvalue reference.
					// 所有其他种类的"引用的引用" (i.e., 组合当中含有lvalue reference) 都会折叠为 lvalue reference.
	}
	if (false || runAll) // forward
	{
		print_header("forward");
		int x = 1;
		testForward(1); // 左右右
		testForward(x); // 左左右
										// 左值和右值都在testForward里变成了左值(v)
	}
	// <functional>
	if (false || runAll) // function
	{
		print_header("function");
		/*  template<typename _Res, typename... _ArgTypes>
        class function<_Res(_ArgTypes...)> */
		std::function<int(int)> fn1 = TestFunction;					// function
		std::function<int(int)> fn2 = &TestFunction;				// function pointer
		std::function<int(int)> fn3 = TestFunctionStruct(); // function object
		std::function<int(int)> fn4 = [](int x)
		{ return x / 4; }; // lambda expression
		INFO_STREAM("function: fn1(60): " << fn1(60));
		INFO_STREAM("function pointer: fn2(60): " << fn2(60));
		INFO_STREAM("function object: fn3(60): " << fn3(60));
		INFO_STREAM("lambda expression: fn4(60): " << fn4(60));
	}
	if (false || runAll) // bind
	{
		print_header("bind");
		// bind function
		auto fn_five = std::bind(TestBind, 10, 2);
		auto fn_half = std::bind(TestBind, std::placeholders::_1, 2);
		INFO_STREAM("fn_five(): " << fn_five());
		INFO_STREAM("fn_half(10): " << fn_half(10));
		// bind class function
		TestBindStruct ten_two {10, 2};
		auto fn_member_fn = std::bind(&TestBindStruct::multiply, std::placeholders::_1);
		INFO_STREAM("fn_member_fn(ten_two): " << fn_member_fn(ten_two));
	}
}