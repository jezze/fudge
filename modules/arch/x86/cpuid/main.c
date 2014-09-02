#include <module.h>
#include "cpuid.h"

void init()
{

    if (!cpuid_check())
        return;

}

void destroy()
{

}

