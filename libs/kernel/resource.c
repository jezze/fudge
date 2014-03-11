#include <kernel.h>
#include "resource.h"

static struct list iterators;
static struct list resources;
static struct resource_iterator root;

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

void resource_init_item(struct resource_item *item, void *data, enum resource_type type)
{

    memory_clear(item, sizeof (struct resource_item));
    list_init_item(&item->item, data);

    item->id.type = type;

}

void resource_init_iterator(struct resource_iterator *iterator, unsigned int (*match)(struct resource_item *item), unsigned int (*read)(struct resource_item *item, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(iterator, sizeof (struct resource_iterator));
    list_init_item(&iterator->item, iterator);

    iterator->match = match;
    iterator->read = read;

}

static unsigned int match(struct resource_item *item)
{

    return 1;

}

static unsigned int read(struct resource_item *item, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, "../\n", 4, offset);

}

void resource_setup()
{

    list_init(&iterators);
    list_init(&resources);
    resource_init_iterator(&root, match, read);
    resource_register_iterator(&root);

}

