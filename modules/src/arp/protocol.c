#include <memory.h>
#include <vfs.h>
#include <net/net.h>
#include <arp/arp.h>

static void handle_read(struct net_protocol *self, struct net_interface *interface, unsigned int count, void *buffer)
{

    struct arp_protocol *protocol = (struct arp_protocol *)self;

    protocol->count = vfs_write(protocol->buffer, 8192, buffer, count, 0);

}

unsigned int read(struct net_protocol *self, struct net_interface *interface, unsigned int offset, unsigned int count, void *buffer)
{

    struct arp_protocol *protocol = (struct arp_protocol *)self;

    return vfs_read(buffer, count, protocol->buffer, protocol->count, offset);

}

unsigned int write(struct net_protocol *self, struct net_interface *interface, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void arp_protocol_init(struct arp_protocol *protocol)
{

    memory_clear(protocol, sizeof (struct arp_protocol));

    protocol->base.name = "arp";
    protocol->base.handle_read = handle_read;
    protocol->base.read = read;
    protocol->base.write = write;

}

