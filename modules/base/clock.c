#include <module.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "clock.h"

static struct interface_node
{

    struct system_group base;
    struct base_clock_interface *interface;
    struct base_device *device;
    struct system_stream timestamp;
    struct system_stream date;
    struct system_stream time;

} inode[8];

static struct system_group root;
static struct system_group dev;
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

    struct interface_node *node = (struct interface_node *)self->parent;
    unsigned int year = node->interface->get_year(node->device) - 1970;
    unsigned int month = node->interface->get_month(node->device);
    unsigned int day = node->interface->get_day(node->device);
    unsigned int hour = node->interface->get_hours(node->device);
    unsigned int minute = node->interface->get_minutes(node->device);
    unsigned int second = node->interface->get_seconds(node->device);
    unsigned int dyear = ((((365 * year) + (year / 4)) - (year / 100)) + (year / 400));
    unsigned int dmonth = isleapyear(year) ? dotm366[month - 1] : dotm365[month - 1];
    unsigned int timestamp = ((dyear + dmonth + day) * 86400) + ((hour * 3600) + (minute * 60) + second);
    char num[32];

    return memory_read(buffer, count, num, ascii_write_value(num, 32, timestamp, 10, 0), offset);

}

static unsigned int date_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;
    char *num = "0000-00-00";

    ascii_write_zerovalue(num, 10, node->interface->get_year(node->device), 10, 4, 0);
    ascii_write_zerovalue(num, 10, node->interface->get_month(node->device), 10, 2, 5);
    ascii_write_zerovalue(num, 10, node->interface->get_day(node->device), 10, 2, 8);

    return memory_read(buffer, count, num, 10, offset);

}

static unsigned int time_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;
    char *num = "00:00:00";

    ascii_write_zerovalue(num, 8, node->interface->get_hours(node->device), 10, 2, 0);
    ascii_write_zerovalue(num, 8, node->interface->get_minutes(node->device), 10, 2, 3);
    ascii_write_zerovalue(num, 8, node->interface->get_seconds(node->device), 10, 2, 6);

    return memory_read(buffer, count, num, 8, offset);

}

static unsigned int find_inode()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!inode[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_inode(struct interface_node *node, struct base_clock_interface *interface, struct base_device *device)
{

    memory_clear(node, sizeof (struct interface_node));
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

    unsigned int index = find_inode();

    if (!index)
        return;

    init_inode(&inode[index], interface, device);
    system_group_add(&dev, &inode[index].base.node);
    system_group_add(&inode[index].base, &inode[index].timestamp.node);
    system_group_add(&inode[index].base, &inode[index].date.node);
    system_group_add(&inode[index].base, &inode[index].time.node);

}

void base_clock_init_interface(struct base_clock_interface *interface, unsigned char (*get_seconds)(struct base_device *device), unsigned char (*get_minutes)(struct base_device *device), unsigned char (*get_hours)(struct base_device *device), unsigned char (*get_weekday)(struct base_device *device), unsigned char (*get_day)(struct base_device *device), unsigned char (*get_month)(struct base_device *device), unsigned short (*get_year)(struct base_device *device))
{

    memory_clear(interface, sizeof (struct base_clock_interface));
    base_init_interface(&interface->base, BASE_INTERFACE_TYPE_OTHER);

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

    memory_clear(inode, sizeof (struct interface_node) * 8);
    system_init_group(&root, "clock");
    system_register_node(&root.node);
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);

}

