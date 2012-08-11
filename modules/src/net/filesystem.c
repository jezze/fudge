#include <memory.h>
#include <modules.h>
#include <net/net.h>

static unsigned int read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int write(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int walk(struct modules_filesystem *self, unsigned int id, unsigned int count, void *buffer)
{

    return 0;

}

void net_filesystem_init(struct modules_filesystem *filesystem, struct modules_driver *driver)
{

    memory_clear(filesystem, sizeof (struct modules_filesystem));

    modules_filesystem_init(filesystem, 0x1001, driver, "net", 0, 0, read, write, 0, walk, 0); 

}

