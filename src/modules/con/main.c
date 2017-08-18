#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "con.h"

static struct system_node root;
static struct system_node clone;

static unsigned int clone_child(struct system_node *self, struct service_state *state, char *path, unsigned int length)
{

    struct list_item *current;

    for (current = root.children.head; current; current = current->next)
    {

        struct system_node *node = current->data;

        if (node == self)
            continue;

        return node->child(node, state, path, length);

    }

    return 0;

}

static unsigned int conctrl_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct con *con = self->resource->data;

    return memory_read(buffer, count, &con->settings, sizeof (struct ctrl_consettings), offset);

}

static unsigned int conctrl_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct con *con = self->resource->data;

    return memory_write(&con->settings, sizeof (struct ctrl_consettings), buffer, count, offset);

}

static unsigned int condata_open(struct system_node *self, struct service_state *state)
{

    struct con *con = self->resource->data;

    list_add(&con->states, &state->item);
    con->open();

    return (unsigned int)self;

}

static unsigned int condata_close(struct system_node *self, struct service_state *state)
{

    struct con *con = self->resource->data;

    list_remove(&con->states, &state->item);
    con->close();

    return (unsigned int)self;

}

static unsigned int condata_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct con *con = self->resource->data;

    return con->write(buffer, count);

}

void con_init(struct con *con, void (*open)(void), void (*close)(void), unsigned int (*write)(void *buffer, unsigned int count))
{

    ctrl_setconsettings(&con->settings, 0, 0, 0);
    resource_init(&con->resource, RESOURCE_CON, con);
    system_initresourcenode(&con->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "con", &con->resource);
    system_initresourcenode(&con->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl", &con->resource);
    system_initresourcenode(&con->data, SYSTEM_NODETYPE_NORMAL, "data", &con->resource);

    con->open = open;
    con->close = close;
    con->write = write;
    con->ctrl.read = conctrl_read;
    con->ctrl.write = conctrl_write;
    con->data.open = condata_open;
    con->data.close = condata_close;
    con->data.read = system_readtask;
    con->data.write = condata_write;

}

void con_register(struct con *con)
{

    system_addchild(&con->root, &con->ctrl);
    system_addchild(&con->root, &con->data);
    system_addchild(&root, &con->root);

}

void con_unregister(struct con *con)
{

    system_removechild(&con->root, &con->ctrl);
    system_removechild(&con->root, &con->data);
    system_removechild(&root, &con->root);

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "con");
    system_initnode(&clone, SYSTEM_NODETYPE_GROUP, "clone");

    clone.child = clone_child;

    system_addchild(&root, &clone);

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

