#include <module.h>
#include <kernel/rendezvous.h>
#include <base/base.h>
#include <base/terminal.h>
#include "driver.h"

void init()
{

    uart_driver_init();

}

void destroy()
{

    uart_driver_destroy();

}

