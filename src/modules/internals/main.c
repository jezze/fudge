#include <fudge.h>
#include <kernel.h>
#include "internals.h"

void module_init(void)
{

    internals_initbackend();
    internals_initprotocol();

}

