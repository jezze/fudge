#include <fudge.h>
#include <kernel.h>
#include "system.h"

static struct system_node *open(struct system_node *self, struct service_state *state)
{

    list_add(&self->states, &state->item);

    return self;

}

static struct system_node *close(struct system_node *self, struct service_state *state)
{

    list_remove(&self->states, &state->item);

    return self;

}

static unsigned int readgroup(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct record record;

    if (!current)
        return 0;

    record.id = (unsigned int)current;
    record.size = 0;
    record.length = memory_read(record.name, RECORD_NAMESIZE, current->name, ascii_length(current->name), 0);

    if (current->type == SYSTEM_NODETYPE_MULTIGROUP)
    {

        char num[FUDGE_NSIZE];

        record.length += memory_write(record.name, RECORD_NAMESIZE, ":", 1, record.length);
        record.length += memory_write(record.name, RECORD_NAMESIZE, num, ascii_wvalue(num, FUDGE_NSIZE, current->index, 10), record.length);

    }

    return memory_read(buffer, count, &record, sizeof (struct record), offset);

}

static unsigned int read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    switch (self->type)
    {

    case SYSTEM_NODETYPE_GROUP:
    case SYSTEM_NODETYPE_MULTIGROUP:
        return readgroup(self, current, state, buffer, count, offset);

    }

    return 0;

}

static unsigned int write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return 0;

}

static unsigned int seek(struct system_node *self, struct service_state *state, unsigned int offset)
{

    return offset;

}

void system_addchild(struct system_node *group, struct system_node *node)
{

    struct list_item *current;

    if (node->type == SYSTEM_NODETYPE_MULTIGROUP)
    {

        unsigned int length0 = ascii_length(node->name);

        spinlock_acquire(&group->children.spinlock);

        for (current = group->children.head; current; current = current->next)
        {

            struct system_node *n = current->data;
            unsigned int length1 = ascii_length(n->name);

            if (length0 != length1 || !memory_match(n->name, node->name, length0))
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

void system_movechild(struct system_node *to, struct system_node *from, struct system_node *node)
{

    list_move(&to->children, &from->children, &node->item);

    node->parent = to;

}

void system_initnode(struct system_node *node, unsigned int type, char *name)
{

    list_inititem(&node->item, node);

    node->type = type;
    node->name = name;
    node->operations.open = open;
    node->operations.close = close;
    node->operations.read = read;
    node->operations.write = write;
    node->operations.seek = seek;

}

void module_init(void)
{

    system_initbackend();
    system_initprotocol();

}

