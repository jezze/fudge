#include <fudge.h>
#include <kernel.h>
#include "system.h"

static struct system_node root;
static struct service service;

static struct system_node *getnode(unsigned int id)
{

    return (struct system_node *)id;

}

static unsigned int service_root(void)
{

    return (unsigned int)&root;

}

static unsigned int service_parent(unsigned int id)
{

    struct system_node *node = getnode(id);

    return (unsigned int)node->parent;

}

static unsigned int service_child(unsigned int id, char *path, unsigned int length)
{

    struct system_node *node = getnode(id);
    struct system_node *child = 0;
    struct list_item *current;

    spinlock_acquire(&node->children.spinlock);

    for (current = node->children.head; current; current = current->next)
    {

        struct system_node *cnode = current->data;
        unsigned int length0 = cstring_length(cnode->name);

        if (cnode->type == SYSTEM_NODETYPE_MULTIGROUP)
        {

            unsigned int colon = buffer_findbyte(path, length, '.');
            unsigned int val;

            if (length0 != colon)
                continue;

            if (!buffer_match(cnode->name, path, colon))
                continue;

            val = cstring_read_value(path + colon + 1, length - colon - 1, 10);

            if (val != cnode->index)
                continue;

        }

        else
        {

            if (length0 != length)
                continue;

            if (!buffer_match(cnode->name, path, length))
                continue;

        }

        child = cnode;

        break;

    }

    spinlock_release(&node->children.spinlock);

    return (unsigned int)child;

}

static unsigned int service_create(unsigned int id, char *name, unsigned int length)
{

    struct system_node *node = getnode(id);

    return (node->operations.create) ? node->operations.create(name, length) : 0;

}

static unsigned int service_destroy(unsigned int id)
{

    struct system_node *node = getnode(id);

    return (node->operations.destroy) ? node->operations.destroy() : 0;

}

static unsigned int service_stat(unsigned int id, struct record *record)
{

    struct system_node *node = getnode(id);

    if (node)
    {

        record->id = (unsigned int)node;
        record->size = 0;
        record->type = RECORD_TYPE_NORMAL;
        record->length = buffer_read(record->name, RECORD_NAMESIZE, node->name, cstring_length(node->name), 0);

        if (node->type == SYSTEM_NODETYPE_GROUP)
        {

            record->type = RECORD_TYPE_DIRECTORY;

        }

        if (node->type == SYSTEM_NODETYPE_MULTIGROUP)
        {

            record->type = RECORD_TYPE_DIRECTORY;
            record->length += cstring_write_fmt1(record->name, RECORD_NAMESIZE, ".%u", record->length, &node->index);

        }

        return 1;

    }

    return 0;

}

static unsigned int service_list(unsigned int id, unsigned int offset, unsigned int count, struct record *records)
{

    struct system_node *node = getnode(id);

    if (node->type == SYSTEM_NODETYPE_GROUP || node->type == SYSTEM_NODETYPE_MULTIGROUP)
    {

        struct list_item *current;
        unsigned int n = 0;

        spinlock_acquire(&node->children.spinlock);

        for (current = node->children.head; current; current = current->next)
        {

            if (offset)
            {

                offset--;

                continue;

            }

            else
            {

                struct system_node *child = current->data;
                struct record *record = &records[n];

                record->id = (unsigned int)child;
                record->size = 0;
                record->type = RECORD_TYPE_NORMAL;
                record->length = buffer_read(record->name, RECORD_NAMESIZE, child->name, cstring_length(child->name), 0);

                if (child->type == SYSTEM_NODETYPE_GROUP)
                {

                    record->type = RECORD_TYPE_DIRECTORY;

                }

                if (child->type == SYSTEM_NODETYPE_MULTIGROUP)
                {

                    record->type = RECORD_TYPE_DIRECTORY;
                    record->length += cstring_write_fmt1(record->name, RECORD_NAMESIZE, ".%u", record->length, &child->index);

                }

                if (++n >= count)
                    break;

            }

        }

        spinlock_release(&node->children.spinlock);

        return n;

    }

    return 0;

}

static unsigned int service_read(unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    struct system_node *node = getnode(id);

    return (node->operations.read) ? node->operations.read(buffer, count, offset) : 0;

}

static unsigned int service_write(unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    struct system_node *node = getnode(id);

    return (node->operations.write) ? node->operations.write(buffer, count, offset) : 0;

}

static unsigned int service_map(unsigned int id)
{

    return 0;

}

static unsigned int service_link(unsigned int id, unsigned int target, unsigned int source)
{

    struct system_node *node = getnode(id);

    kernel_addlink(&node->links, target, source);

    return id;

}

static unsigned int service_unlink(unsigned int id, unsigned int target)
{

    struct system_node *node = getnode(id);

    kernel_removelink(&node->links, target);

    return id;

}

static unsigned int onwalkrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_walkrequest *walkrequest = data;
    struct {struct event_walkresponse walkresponse;} message;

    message.walkresponse.session = walkrequest->session;
    message.walkresponse.id = service_findpath(&service, (walkrequest->parent) ? walkrequest->parent : service_root(), (char *)(walkrequest + 1), walkrequest->length);

    return kernel_place(service.id, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &message);

}

static unsigned int onlistrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_listrequest *listrequest = data;
    struct {struct event_listresponse listresponse; struct record records[8];} message;

    message.listresponse.session = listrequest->session;
    message.listresponse.nrecords = service_list(listrequest->id, listrequest->offset, (listrequest->nrecords > 8) ? 8 : listrequest->nrecords, message.records);

    return kernel_place(service.id, source, EVENT_LISTRESPONSE, sizeof (struct event_listresponse) + sizeof (struct record) * message.listresponse.nrecords, &message);

}

static unsigned int onreadrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_readrequest *readrequest = data;
    struct {struct event_readresponse readresponse; char data[64];} message;

    message.readresponse.session = readrequest->session;
    message.readresponse.count = service_read(readrequest->id, message.data, (readrequest->count > 64) ? 64 : readrequest->count, readrequest->offset);

    return kernel_place(service.id, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + message.readresponse.count, &message);

}

static unsigned int onwriterequest(unsigned int source, unsigned int count, void *data)
{

    struct event_writerequest *writerequest = data;
    struct {struct event_writeresponse writeresponse;} message;

    message.writeresponse.session = writerequest->session;
    message.writeresponse.count = service_write(writerequest->id, writerequest + 1, writerequest->count, writerequest->offset);

    return kernel_place(service.id, source, EVENT_WRITERESPONSE, sizeof (struct event_writeresponse), &message);

}

static unsigned int onlinkrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_linkrequest *linkrequest = data;

    service_link(linkrequest->id, source, 0);

    return kernel_place(service.id, source, EVENT_LINKRESPONSE, 0, 0);

}

static unsigned int onunlinkrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_unlinkrequest *unlinkrequest = data;

    service_unlink(unlinkrequest->id, source);

    return kernel_place(service.id, source, EVENT_UNLINKRESPONSE, 0, 0);

}

static unsigned int service_notify(unsigned int id, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct system_node *node = getnode(id);

    switch (event)
    {

    case EVENT_WALKREQUEST:
        return onwalkrequest(source, count, data);

    case EVENT_LISTREQUEST:
        return onlistrequest(source, count, data);

    case EVENT_READREQUEST:
        return onreadrequest(source, count, data);

    case EVENT_WRITEREQUEST:
        return onwriterequest(source, count, data);

    case EVENT_LINKREQUEST:
        return onlinkrequest(source, count, data);

    case EVENT_UNLINKREQUEST:
        return onunlinkrequest(source, count, data);

    default:
        return (node->operations.notify) ? node->operations.notify(source, event, count, data) : 0;

    }

    return 0;

}

void system_registernode(struct system_node *node)
{

    system_addchild(&root, node);

}

void system_unregisternode(struct system_node *node)
{

    system_removechild(&root, node);

}

void system_addchild(struct system_node *group, struct system_node *node)
{

    struct list_item *current;

    if (node->type == SYSTEM_NODETYPE_MULTIGROUP)
    {

        unsigned int length0 = cstring_length(node->name);

        spinlock_acquire(&group->children.spinlock);

        for (current = group->children.head; current; current = current->next)
        {

            struct system_node *n = current->data;
            unsigned int length1 = cstring_length(n->name);

            if (length0 != length1 || !buffer_match(n->name, node->name, length0))
                continue;

            node->index++;

        }

        spinlock_release(&group->children.spinlock);

    }

    list_add(&group->children, &node->item);

    node->parent = group;

}

void system_removechild(struct system_node *group, struct system_node *node)
{

    list_remove(&group->children, &node->item);

    node->parent = 0;

}

void system_initnode(struct system_node *node, unsigned int type, char *name)
{

    list_inititem(&node->item, node);
    list_init(&node->children);
    list_init(&node->links);

    node->type = type;
    node->name = name;
    node->index = 0;
    node->parent = 0;
    node->operations.create = 0;
    node->operations.destroy = 0;
    node->operations.read = 0;
    node->operations.write = 0;
    node->operations.notify = 0;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "FUDGE_ROOT");
    service_init(&service, "system", 501, service_root, service_parent, service_child, service_create, service_destroy, service_stat, service_list, service_read, service_write, service_map, service_link, service_unlink, service_notify);
    resource_register(&service.resource);
    kernel_announce(service.id, &service, service_root());

}

