#include <module.h>
#include <kernel/rendezvous.h>
#include <base/base.h>
#include <base/timer.h>
#include "driver.h"

void init()
{

    pit_driver_init();

}

void destroy()
{

    pit_driver_destroy();

}

