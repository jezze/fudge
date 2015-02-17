#include <fudge.h>
#include "cpuid.h"

void module_init()
{

    if (!cpuid_check())
        return;

}

void module_register()
{

}

void module_unregister()
{

}

