#include <exception>
#include <assert.h>
#include "fun.h"
#include "../include/helper.hh"

int TestStaticVarInFunction()
{
    static int s_id{0};
    return ++s_id;
}
class TestStaticVarInClass
{
public:
    TestStaticVarInClass(int a) { _var += a; }
    static int _var;
};
int TestStaticVarInClass::_var = 0;
class TestStaticFunInClass : public TestStaticVarInClass
{
public:
    TestStaticFunInClass(int a) : TestStaticVarInClass(a) {}
    static int getStaticVar() { return _var; } // 只能访问静态变量
    static void IncreateStaicMem();
};
void TestStaticFunInClass::IncreateStaicMem() // 不需要加static
{
    ++_var;
}

struct TestWithoutExplict
{
    TestWithoutExplict(int) {}
    operator bool() { return true; }
};
struct TestWithExplict
{
    explicit TestWithExplict(int) {}
    explicit operator bool() { return true; }
};

class TestMutable
{
public:
    TestMutable() {}
    TestMutable(int val1, int val2) : var1(val1), var2(val2) {}
    void ModifyMemberVar(int val1, int val2) const;
    bool operator>(const TestMutable &a) { return var2 > a.var2; }
    void display() { INFO_STREAM("var1: " << var1 << ", " << var2); }

private:
    int var1 = 0;
    mutable int var2 = 0;
};
void TestMutable::ModifyMemberVar(int val1, int val2) const
{
    // var1 = val1;
    var2 = val2;
}

template <class T>
T TestTemplateSelectMax(T a, T b)
{
    return (a > b ? a : b);
}

struct TestException : std::exception
{
    const char *what() const noexcept { return "Ooops! Exception!"; }
};

int main()
{
    bool runAll = true;
    if (false || runAll) // static
    {
        print_header("Static");
        // 静态成员变量，所有类实例共享一个(存储在ELF文件的.bss或者.data段)
        TestStaticVarInClass a(1);
        INFO_STREAM("Construct TestStaticVarInClass a(1): " << a._var); // 1
        TestStaticVarInClass b(1);
        // 静态成员函数, 静态成员函数并不具体作用于某个对象。
        INFO_STREAM("Construct TestStaticVarInClass b(1): " << a._var); // 2
        TestStaticFunInClass::IncreateStaicMem();
        INFO_STREAM("TestStaticFunInClass::getStaticVar(): " << TestStaticFunInClass::getStaticVar()); // 2
        // 全局静态变量，任何包含该头文件的源文件都会在各自的编译单元中创建一个独立的该变量的副本。
        IncreaseStaticVar1();                  // 2
        IncreaseStaticVar1();                  // 3
        IncreaseStaticVar2();                  // 2
        INFO_STREAM("Main: A: " << staticVar); // 1
        // 局部静态变量
        INFO_STREAM("Call TestStaticVarInFunction(): " << TestStaticVarInFunction()); // 1
        INFO_STREAM("Call TestStaticVarInFunction(): " << TestStaticVarInFunction()); // 2
    }
    if (false || runAll) // extern
    {
        print_header("extern");
        extern int ExternVar;
        INFO_STREAM("ExternVar: " << ExternVar);
    }
    if (false || runAll) // const & constexpr
    {
        print_header("const & constexpr");
        const int MAX_BUFFER = 1024;     // 传统常量
        constexpr double PI = 3.1415926; // C++11编译期常量
        // 如果类型比较大，参数传递可以用引用，const避免对变量的修改
        std::string var = "Hello, const";
        auto printVar = [](const std::string &var)
        {
            INFO_STREAM("const reference: " << var);
        };
        printVar(var);
    }
    if (false || runAll) // explicit
    {
        print_header("explicit");
        TestWithoutExplict a1 = 1;
        TestWithoutExplict a2(1);
        TestWithoutExplict a3 = (TestWithoutExplict)1;
        if (a1)
        {
        }
        bool ba = a1; // OK: copy-initialization selects A::operator bool()
        // TestWithExplict b1 = 1;  // error: copy-initialization does not consider B::B(int)
        TestWithExplict b2(1);
        TestWithExplict b3 = (TestWithExplict)1;
        if (b2)
        {
        }
        // bool bb = b2; // error: copy-initialization does not consider B::operator bool()
    }
    if (false || runAll) // inline
    {
        int b = sum(1, 2);
    }
    if (false || runAll) // mutable
    {
        print_header("mutable");
        // mutable也是为了突破const的限制而设置的。被mutable修饰的变量，将永远处于可变的状态，即使在一个const函数中。
        TestMutable a = TestMutable();
        a.ModifyMemberVar(1, 2);
    }
    if (false || runAll) // template
    {
        print_header("template");
        TestMutable a = TestMutable(1, 2);
        TestMutable b = TestMutable(1, 3);
        TestMutable c = TestTemplateSelectMax<TestMutable>(a, b); // select b
        c.display();
    }
    if (false || runAll) // try & catch & raise
    {
        print_header("try & catch & raise");
        try
        {
            // assert(1<0);
            TestException e;
            throw e;
        }
        catch (std::exception &ex)
        {
            INFO_STREAM("" << ex.what());
        }
    }
    return 0;
}