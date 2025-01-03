#include "fun.h"
#include "../include/helper.hh"

void IncreaseStaticVar1()
{
    staticVar += 1;
    INFO_STREAM("IncreaseStaticVar1: A: "<<staticVar);
}