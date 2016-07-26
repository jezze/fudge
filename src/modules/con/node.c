#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "con.h"

static struct con con;

void module_init(void)
{

    con_init(&con);

}

void module_register(void)
{

    con_register(&con);

}

void module_unregister(void)
{

    con_unregister(&con);

}

