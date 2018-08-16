#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "pipe.h"

static struct system_node root;
static struct system_node used;
static struct system_node free;
static struct system_node clone;

static struct system_node *clone_open(struct system_node *self, struct service_state *state)
{

    struct system_node *node = list_pickhead(&free.children)->data;

    return node->operations.open(node, state);

}

void pipe_use(struct pipe *pipe)
{

    ring_reset(&pipe->ring);
    system_addchild(&used, &pipe->root);

}

void pipe_free(struct pipe *pipe)
{

    system_movechild(&free, &used, &pipe->root);

}

void pipe_init(struct pipe *pipe)
{

    ring_init(&pipe->ring, FUDGE_BSIZE, pipe->data);
    system_initnode(&pipe->root, SYSTEM_NODETYPE_MULTIGROUP, "pipe");
    system_initnode(&pipe->idata, SYSTEM_NODETYPE_NORMAL, "idata");
    system_initnode(&pipe->odata, SYSTEM_NODETYPE_NORMAL, "odata");

}

void pipe_register(struct pipe *pipe)
{

    system_addchild(&pipe->root, &pipe->idata);
    system_addchild(&pipe->root, &pipe->odata);
    system_addchild(&free, &pipe->root);

}

void pipe_unregister(struct pipe *pipe)
{

    system_removechild(&free, &pipe->root);
    system_removechild(&pipe->root, &pipe->idata);
    system_removechild(&pipe->root, &pipe->odata);

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "pipe");
    system_initnode(&used, SYSTEM_NODETYPE_GROUP, "used");
    system_initnode(&free, SYSTEM_NODETYPE_GROUP, "free");
    system_initnode(&clone, SYSTEM_NODETYPE_NORMAL, "clone");

    clone.operations.open = clone_open;

}

void module_register(void)
{

    system_addchild(&root, &used);
    system_addchild(&root, &free);
    system_addchild(&root, &clone);
    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);
    system_removechild(&root, &used);
    system_removechild(&root, &free);
    system_removechild(&root, &clone);

}

