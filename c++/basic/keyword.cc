#include "fun.h"
#include "../include/helper.hh"

int main()
{
    {// static
        print_header("Keyword: Static");
        IncreaseStaticVar1();
        IncreaseStaticVar1();
        IncreaseStaticVar2();
        INFO_STREAM("Main: A: "<<staticVar);
    }
    return 0;
}