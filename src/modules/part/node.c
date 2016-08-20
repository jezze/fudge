#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "part.h"

static struct part part;

void module_init(void)
{

    part_init(&part);

}

void module_register(void)
{

    part_register(&part);

}

void module_unregister(void)
{

    part_unregister(&part);

}

