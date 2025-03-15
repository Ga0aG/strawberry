#include "fun.h"
#include "../include/helper.hh"
int ExternVar = 10;
void IncreaseStaticVar1()
{
    staticVar += 1;
    INFO_STREAM("IncreaseStaticVar1: A: "<<staticVar);
}