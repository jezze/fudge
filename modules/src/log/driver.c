#include <memory.h>
#include <modules.h>
#include <log/log.h>

static void start(struct modules_driver *self)
{

}

void log_driver_init(struct log_driver *driver)
{

    memory_clear(driver, sizeof (struct log_driver));

    modules_driver_init(&driver->base, LOG_DRIVER_TYPE, "log", start, 0, 0);

    driver->buffer.count = 0;

}

