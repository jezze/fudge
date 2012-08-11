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

void net_filesystem_init(struct net_filesystem *filesystem, struct net_driver *driver)
{

    memory_clear(filesystem, sizeof (struct net_filesystem));

    modules_filesystem_init(&filesystem->base, 0x1001, &driver->base, "net", 0, 0, read, write, 0, walk, 0); 

}

