#include <fudge.h>
#include "cpuid.h"

void module_init(void)
{

    if (!cpuid_check())
        return;

}

