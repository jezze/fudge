#include <fudge.h>
#include <net.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include "ipv6.h"

static struct ethernet_hook ethernethook;

static void ethernethook_notify(struct ethernet_header *ethernetheader, void *buffer, unsigned int count)
{

    struct ipv6_header *header = buffer;
    unsigned int length = (header->length[0] << 8) | header->length[1];
    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_IPV6HOOK)))
    {

        struct ipv6_hook *hook = current->data;

        if (hook->id == header->next)
            hook->notify(header, header + 1, length * 8);

    }

    kernel_notify(&ethernethook.data.links, EVENT_DATA, buffer, count);

}

void ipv6_registerhook(struct ipv6_hook *hook)
{

    resource_register(&hook->resource);
    system_addchild(&hook->root, &hook->data);
    system_addchild(&ethernethook.root, &hook->root);

}

void ipv6_unregisterhook(struct ipv6_hook *hook)
{

    resource_unregister(&hook->resource);
    system_removechild(&hook->root, &hook->data);
    system_removechild(&ethernethook.root, &hook->root);

}

void ipv6_inithook(struct ipv6_hook *hook, char *name, unsigned char id, void (*notify)(struct ipv6_header *ipv6header, void *buffer, unsigned int count))
{

    resource_init(&hook->resource, RESOURCE_IPV4HOOK, hook);
    system_initnode(&hook->root, SYSTEM_NODETYPE_GROUP, name);
    system_initnode(&hook->data, SYSTEM_NODETYPE_NORMAL, "data");

    hook->id = id;
    hook->notify = notify;

}

void module_init(void)
{

    ethernet_inithook(&ethernethook, "ipv6", 0x86DD, ethernethook_notify);

}

void module_register(void)
{

    ethernet_registerhook(&ethernethook);

}

void module_unregister(void)
{

    ethernet_unregisterhook(&ethernethook);

}

