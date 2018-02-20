#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "gdb.h"

void module_init(void)
{

    set_debug_traps();
    breakpoint();

}

