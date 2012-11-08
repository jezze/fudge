#include <memory.h>
#include <vfs.h>
#include <base/base.h>
#include <tty/tty.h>

void tty_init_driver(struct tty_driver *driver, char *cwdname, unsigned int cwdcount)
{

    memory_clear(driver, sizeof (struct tty_driver));

    base_init_driver(&driver->base, TTY_DRIVER_TYPE, "tty", 0, 0, 0);

    driver->cwdcount = cwdcount;
    memory_write(driver->cwdname, TTY_CWD_SIZE, cwdname, cwdcount, 0);

}

