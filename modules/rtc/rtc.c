#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/rtc/rtc.h>

static struct rtc_device device;

static unsigned char convert(unsigned char num)
{

    return ((num & 0xF0) >> 1) + ((num & 0xF0) >> 3) + (num & 0x0F);

}

static unsigned char get_value(unsigned int type)
{

//    do { io_outb(RTC_PORT_WRITE, 0x0A); }
//    while (io_inb(RTC_PORT_READ) != 0x80);

    io_outb(RTC_PORT_WRITE, type);

    return convert(io_inb(RTC_PORT_READ));

}

static unsigned int rtc_device_read_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    string_write_format(buffer, "20%d-%d-%d %d:%d:%d", get_value(RTC_FLAG_YEAR), get_value(RTC_FLAG_MONTH), get_value(RTC_FLAG_DAY), get_value(RTC_FLAG_HOURS), get_value(RTC_FLAG_MINUTES), get_value(RTC_FLAG_SECONDS));

    return string_length(buffer);

}

static struct vfs_node *rtc_device_view_find_node(struct vfs_view *self, char *name)
{

    if (!string_compare("date", name))
        return &device.read;

    return 0;

}

static struct vfs_node *rtc_device_view_walk(struct vfs_view *self, unsigned int index)
{

    if (index == 0)
        return &device.read;

    return 0;

}

static char *rtc_device_view_get_name(struct vfs_view *self, struct vfs_node *node)
{

    if (node->id == 0)
        return "date";

    return 0;

}

void rtc_device_init(struct rtc_device *device)
{

    modules_device_init(&device->base, RTC_DEVICE_TYPE);

    vfs_node_init(&device->read, 0, 0, 0, rtc_device_read_read, 0);
    vfs_view_init(&device->view, "rtc", rtc_device_view_find_node, rtc_device_view_walk, rtc_device_view_get_name);

    device->base.module.view = &device->view;

}

void init()
{

    rtc_device_init(&device);

    modules_register_device(&device.base);

}

void destroy()
{

    modules_unregister_device(&device.base);

}

