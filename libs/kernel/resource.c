#include <kernel.h>
#include "resource.h"

static struct list iterators;
static struct list resources;
static struct resource_iterator root;

struct resource_item *resource_find_item(enum resource_type type)
{

    struct list_item *current;

    for (current = resources.head; current; current = current->next)
    {

        struct resource_item *item = current->data;

        if (item->id.type == type)
            return item;

    }

    return 0;

}

struct resource_iterator *resource_find_iterator(enum resource_type type)
{

    return &root;

}

void resource_register_item(struct resource_item *item)
{

    list_add(&resources, &item->item);

}

void resource_register_iterator(struct resource_iterator *iterator)
{

    list_add(&iterators, &iterator->item);

}

void resource_init_item(struct resource_item *item, void *data, enum resource_type type, unsigned int size, const char *text)
{

    memory_clear(item, sizeof (struct resource_item));
    list_init_item(&item->item, data);

    item->id.type = type;
    item->id.size = size;
    item->id.text = text;

}

void resource_init_iterator(struct resource_iterator *iterator, unsigned int (*read)(unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(iterator, sizeof (struct resource_iterator));
    list_init_item(&iterator->item, iterator);

    iterator->read = read;

}

static unsigned int read(unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "../\n", 4, offset);

}

void resource_setup()
{

    list_init(&iterators);
    list_init(&resources);
    resource_init_iterator(&root, read);
    resource_register_iterator(&root);

}

