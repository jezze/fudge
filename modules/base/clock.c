#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "clock.h"

static struct system_group root;
static unsigned int dotm365[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
static unsigned int dotm366[13] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 365};

static unsigned int isleapyear(unsigned short year)
{

    if ((year % 4) != 0)
        return 0;

    if ((year % 100) == 0)
        return ((year % 400) == 0);

    return 1;

}

static unsigned int timestamp_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_clock_node *node = (struct base_clock_node *)self->parent;
    unsigned int year = node->interface->get_year(node->device->bus, node->device->id) - 1970;
    unsigned int month = node->interface->get_month(node->device->bus, node->device->id);
    unsigned int day = node->interface->get_day(node->device->bus, node->device->id);
    unsigned int hour = node->interface->get_hours(node->device->bus, node->device->id);
    unsigned int minute = node->interface->get_minutes(node->device->bus, node->device->id);
    unsigned int second = node->interface->get_seconds(node->device->bus, node->device->id);
    unsigned int dyear = ((((365 * year) + (year / 4)) - (year / 100)) + (year / 400));
    unsigned int dmonth = isleapyear(year) ? dotm366[month - 1] : dotm365[month - 1];
    unsigned int timestamp = ((dyear + dmonth + day) * 86400) + ((hour * 3600) + (minute * 60) + second);
    char num[32];

    return memory_read(buffer, count, num, ascii_write_value(num, 32, timestamp, 10, 0), offset);

}

static unsigned int date_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_clock_node *node = (struct base_clock_node *)self->parent;
    char *num = "0000-00-00";

    ascii_write_zerovalue(num, 10, node->interface->get_year(node->device->bus, node->device->id), 10, 4, 0);
    ascii_write_zerovalue(num, 10, node->interface->get_month(node->device->bus, node->device->id), 10, 2, 5);
    ascii_write_zerovalue(num, 10, node->interface->get_day(node->device->bus, node->device->id), 10, 2, 8);

    return memory_read(buffer, count, num, 10, offset);

}

static unsigned int time_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_clock_node *node = (struct base_clock_node *)self->parent;
    char *num = "00:00:00";

    ascii_write_zerovalue(num, 8, node->interface->get_hours(node->device->bus, node->device->id), 10, 2, 0);
    ascii_write_zerovalue(num, 8, node->interface->get_minutes(node->device->bus, node->device->id), 10, 2, 3);
    ascii_write_zerovalue(num, 8, node->interface->get_seconds(node->device->bus, node->device->id), 10, 2, 6);

    return memory_read(buffer, count, num, 8, offset);

}

void base_clock_register_interface(struct base_clock_interface *interface)
{

    base_register_interface(&interface->base);

}

void base_clock_register_node(struct base_clock_node *node)
{

    system_group_add(&root, &node->base.node);
    system_group_add(&node->base, &node->timestamp.node);
    system_group_add(&node->base, &node->date.node);
    system_group_add(&node->base, &node->time.node);

}

void base_clock_unregister_interface(struct base_clock_interface *interface)
{

    base_unregister_interface(&interface->base);

}

void base_clock_unregister_node(struct base_clock_node *node)
{

}

void base_clock_init_interface(struct base_clock_interface *interface, unsigned char (*get_seconds)(struct base_bus *bus, unsigned int id), unsigned char (*get_minutes)(struct base_bus *bus, unsigned int id), unsigned char (*get_hours)(struct base_bus *bus, unsigned int id), unsigned char (*get_weekday)(struct base_bus *bus, unsigned int id), unsigned char (*get_day)(struct base_bus *bus, unsigned int id), unsigned char (*get_month)(struct base_bus *bus, unsigned int id), unsigned short (*get_year)(struct base_bus *bus, unsigned int id))
{

    memory_clear(interface, sizeof (struct base_clock_interface));
    base_init_interface(&interface->base, BASE_INTERFACE_TYPE_CLOCK);

    interface->get_seconds = get_seconds;
    interface->get_minutes = get_minutes;
    interface->get_hours = get_hours;
    interface->get_weekday = get_weekday;
    interface->get_day = get_day;
    interface->get_month = get_month;
    interface->get_year = get_year;

}

void base_clock_init_node(struct base_clock_node *node, struct base_device *device, struct base_clock_interface *interface)
{

    memory_clear(node, sizeof (struct base_clock_node));
    system_init_group(&node->base, device->bus->name);
    system_init_stream(&node->timestamp, "timestamp");
    system_init_stream(&node->date, "date");
    system_init_stream(&node->time, "time");

    node->device = device;
    node->interface = interface;
    node->timestamp.node.read = timestamp_read;
    node->date.node.read = date_read;
    node->time.node.read = time_read;

}

void base_clock_setup()
{

    system_init_group(&root, "clock");
    system_register_node(&root.node);

}

