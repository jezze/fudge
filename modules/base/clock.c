#include <fudge/module.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "clock.h"

static struct clock_node
{

    struct system_group base;
    struct base_clock_interface *interface;
    struct base_device *device;
    struct system_stream timestamp;
    struct system_stream date;
    struct system_stream time;

} node[8];

static struct system_group root;
static struct system_group dev;

static unsigned int timestamp_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int timestamp = 42;
    char num[32];

    return memory_read(buffer, count, num, memory_write_number(num, 32, timestamp, 10, 0), offset);

}

static unsigned int date_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct clock_node *node = (struct clock_node *)self->parent;
    char *num = "0000-00-00";

    memory_write_paddednumber(num, 10, node->interface->get_year(node->device), 10, 4, 0);
    memory_write_paddednumber(num, 10, node->interface->get_month(node->device), 10, 2, 5);
    memory_write_paddednumber(num, 10, node->interface->get_day(node->device), 10, 2, 8);

    return memory_read(buffer, count, num, 10, offset);

}

static unsigned int time_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct clock_node *node = (struct clock_node *)self->parent;
    char *num = "00:00:00";

    memory_write_paddednumber(num, 8, node->interface->get_hours(node->device), 10, 2, 0);
    memory_write_paddednumber(num, 8, node->interface->get_minutes(node->device), 10, 2, 3);
    memory_write_paddednumber(num, 8, node->interface->get_seconds(node->device), 10, 2, 6);

    return memory_read(buffer, count, num, 8, offset);

}

static unsigned int find_node()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!node[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_node(struct clock_node *node, struct base_clock_interface *interface, struct base_device *device)
{

    memory_clear(node, sizeof (struct clock_node));
    system_init_group(&node->base, device->module.name);
    system_init_stream(&node->timestamp, "timestamp");
    system_init_stream(&node->date, "date");
    system_init_stream(&node->time, "time");

    node->interface = interface;
    node->device = device;
    node->timestamp.node.read = timestamp_read;
    node->date.node.read = date_read;
    node->time.node.read = time_read;

}

void base_clock_register_interface(struct base_clock_interface *interface, struct base_device *device)
{

    unsigned int index = find_node();

    if (!index)
        return;

    init_node(&node[index], interface, device);
    system_group_add(&dev, &node[index].base.node);
    system_group_add(&node[index].base, &node[index].timestamp.node);
    system_group_add(&node[index].base, &node[index].date.node);
    system_group_add(&node[index].base, &node[index].time.node);

}

void base_clock_init_interface(struct base_clock_interface *interface, unsigned int (*get_timestamp)(struct base_device *device), unsigned char (*get_seconds)(struct base_device *device), unsigned char (*get_minutes)(struct base_device *device), unsigned char (*get_hours)(struct base_device *device), unsigned char (*get_weekday)(struct base_device *device), unsigned char (*get_day)(struct base_device *device), unsigned char (*get_month)(struct base_device *device), unsigned short (*get_year)(struct base_device *device))
{

    memory_clear(interface, sizeof (struct base_clock_interface));

    interface->get_timestamp = get_timestamp;
    interface->get_seconds = get_seconds;
    interface->get_minutes = get_minutes;
    interface->get_hours = get_hours;
    interface->get_weekday = get_weekday;
    interface->get_day = get_day;
    interface->get_month = get_month;
    interface->get_year = get_year;

}

void base_clock_setup()
{

    memory_clear(node, sizeof (struct clock_node) * 8);
    system_init_group(&root, "clock");
    system_register_node(&root.node);
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);

}

