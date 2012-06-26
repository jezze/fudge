#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/modules.h>
#include <modules/nodefs/nodefs.h>
#include <modules/log/log.h>

static struct log_driver driver;
static struct nodefs_node messages;

static unsigned int messages_read(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    if (offset > driver.buffer.count)
        return 0;

    if (count > driver.buffer.count - offset)
        count = driver.buffer.count - offset;

    memory_copy(buffer, driver.buffer.buffer + offset, count);

    return count;

}

void log_write(unsigned int module, unsigned int count, char *buffer)
{

    char num[32];

    string_write(num, module, 16);

    memory_copy(driver.buffer.buffer + driver.buffer.count, "[", 1);
    driver.buffer.count += 1;

    memory_copy(driver.buffer.buffer + driver.buffer.count, num, string_length(num));
    driver.buffer.count += string_length(num);

    memory_copy(driver.buffer.buffer + driver.buffer.count, "] ", 2);
    driver.buffer.count += 2;

    memory_copy(driver.buffer.buffer + driver.buffer.count, buffer, count);
    driver.buffer.count += count;

}

void init()
{

    log_driver_init(&driver);
    modules_register_driver(&driver.base);

    nodefs_register_node(&messages, "log/messages", &driver.base.base, messages_read, 0);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

