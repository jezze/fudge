#include <lib/call.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/io/io.h>
#include <modules/rtc/rtc.h>

static unsigned char rtc_get(unsigned char type)
{

    io_outb(RTC_PORT_WRITE, type);

    return io_inb(RTC_PORT_READ);

}

static void rtc_ready()
{

//    do { io_outb(RTC_PORT_WRITE, 0x0A); }
//    while (io_inb(RTC_PORT_READ) != 0x80);

}

static unsigned int rtc_device_read(struct vfs_node *node, unsigned int count, void *buffer)
{

    rtc_ready();
/*
    switch (offset)
    {

        case 0x00:

            *(char *)buffer = rtc_get(RTC_FLAG_YEAR);

            break;

        case 0x01:

            *(char *)buffer = rtc_get(RTC_FLAG_MONTH);

            break;

        case 0x02:

            *(char *)buffer = rtc_get(RTC_FLAG_DAY);

            break;

        case 0x03:

            *(char *)buffer = rtc_get(RTC_FLAG_HOURS);

            break;

        case 0x04:

            *(char *)buffer = rtc_get(RTC_FLAG_MINUTES);

            break;

        case 0x05:

            *(char *)buffer = rtc_get(RTC_FLAG_SECONDS);

            break;

        default:

            return 0;

    }
*/
    return 1;

}

void rtc_init()
{

}

