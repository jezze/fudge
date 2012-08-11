#include <memory.h>
#include <string.h>
#include <modules.h>
#include <nodefs/nodefs.h>

static void register_node(struct nodefs_driver *self, struct nodefs_node *node, char *name, struct modules_base *module, unsigned int (*read)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer))
{

    unsigned int i;

    for (i = 0; i < 128; i++)
    {

        if (!self->nodes[i])
        {

            node->name = name;
            node->module = module;
            node->read = read;
            node->write = write;

            self->nodes[i] = node;
            self->nodesCount++;

            break;

        }

    }

}

static void unregister_node(struct nodefs_driver *self, struct nodefs_node *node)
{

    unsigned int i;

    for (i = 0; i < 128; i++)
    {

        if (self->nodes[i] == node)
        {

            self->nodes[i] = 0;
            self->nodesCount--;

            break;

        }

    }

}

void nodefs_driver_init(struct nodefs_driver *driver)
{

    memory_clear(driver, sizeof (struct nodefs_driver));

    modules_driver_init(&driver->base, NODEFS_DRIVER_TYPE, "nodefs", 0, 0, 0);

    driver->register_node = register_node;
    driver->unregister_node = unregister_node;

}

