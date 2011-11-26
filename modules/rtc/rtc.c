#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/stream/stream.h>
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

static unsigned int rtc_device_stream_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    char num[32];

    string_write(buffer, "20");
    string_write_num(num, get_value(RTC_FLAG_YEAR), 10);
    string_write_concat(buffer, num);
    string_write_concat(buffer, "-");
    string_write_num(num, get_value(RTC_FLAG_MONTH), 10);
    string_write_concat(buffer, num);
    string_write_concat(buffer, "-");
    string_write_num(num, get_value(RTC_FLAG_DAY), 10);
    string_write_concat(buffer, num);
    string_write_concat(buffer, " ");
    string_write_num(num, get_value(RTC_FLAG_HOURS), 10);
    string_write_concat(buffer, num);
    string_write_concat(buffer, ":");
    string_write_num(num, get_value(RTC_FLAG_MINUTES), 10);
    string_write_concat(buffer, num);
    string_write_concat(buffer, ":");
    string_write_num(num, get_value(RTC_FLAG_SECONDS), 10);
    string_write_concat(buffer, num);

    return string_length(buffer);

}

void rtc_device_init(struct rtc_device *device)
{

    modules_device_init(&device->base, RTC_DEVICE_TYPE);
    stream_device_init(&device->stream, "rtc", rtc_device_stream_read, 0);

}

void init()
{

    rtc_device_init(&device);

    modules_register_device(&device.stream.base);
    modules_register_device(&device.base);

}

void destroy()
{

    modules_unregister_device(&device.stream.base);
    modules_unregister_device(&device.base);

}

