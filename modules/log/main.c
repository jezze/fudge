#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/modules.h>
#include <modules/nodefs/nodefs.h>
#include <modules/log/log.h>

static struct log_driver driver;
static struct nodefs_node messages;

static unsigned int messages_read(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int messages_write(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void init()
{

    log_driver_init(&driver);
    modules_register_driver(&driver.base);

    nodefs_register_node(&messages, "log/messages", &driver.base.base, messages_read, messages_write);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

