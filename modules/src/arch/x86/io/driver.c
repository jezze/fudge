#include <memory.h>
#include <modules.h>
#include <arch/x86/io/io.h>

void io_driver_init(struct io_driver *driver)
{

    memory_clear(driver, sizeof (struct io_driver));

    modules_driver_init(&driver->base, IO_DRIVER_TYPE, "io", 0, 0, 0);

}

