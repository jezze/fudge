#include <lib/call.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/io/io.h>
#include <modules/rtc/rtc.h>

static struct rtc_device rtcDevice;

static unsigned char rtc_convert(unsigned char num)
{

    return (num / 16) * 10 + (num & 0xF);

}

static unsigned char rtc_get(unsigned int type)
{

//    do { io_outb(RTC_PORT_WRITE, 0x0A); }
//    while (io_inb(RTC_PORT_READ) != 0x80);

    io_outb(RTC_PORT_WRITE, type);

    return rtc_convert(io_inb(RTC_PORT_READ));

}

static unsigned int rtc_device_node_read(struct vfs_node *node, unsigned int count, void *buffer)
{

    char num[32];

    string_copy(buffer, "20");
    string_copy_num(num, rtc_get(RTC_FLAG_YEAR), 10);
    string_concat(buffer, num);
    string_concat(buffer, "-");
    string_copy_num(num, rtc_get(RTC_FLAG_MONTH), 10);
    string_concat(buffer, num);
    string_concat(buffer, "-");
    string_copy_num(num, rtc_get(RTC_FLAG_DAY), 10);
    string_concat(buffer, num);
    string_concat(buffer, " ");
    string_copy_num(num, rtc_get(RTC_FLAG_HOURS), 10);
    string_concat(buffer, num);
    string_concat(buffer, ":");
    string_copy_num(num, rtc_get(RTC_FLAG_MINUTES), 10);
    string_concat(buffer, num);
    string_concat(buffer, ":");
    string_copy_num(num, rtc_get(RTC_FLAG_SECONDS), 10);
    string_concat(buffer, num);

    return string_length(buffer);

}

void rtc_init()
{

    rtcDevice.base.module.type = MODULES_TYPE_DEVICE;
    rtcDevice.base.type = 2000;
    string_copy(rtcDevice.base.name, "rtc");
    rtcDevice.base.node.operations.read = rtc_device_node_read;
    modules_register_device(&rtcDevice.base);

}

