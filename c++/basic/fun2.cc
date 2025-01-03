#include "fun.h"
#include "../include/helper.hh"

void IncreaseStaticVar2()
{
    staticVar += 1;
    INFO_STREAM("IncreaseStaticVar2: A: "<<staticVar);
}