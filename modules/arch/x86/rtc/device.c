#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <modules/rtc/rtc.h>

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

void rtc_device_init(struct rtc_device *device)
{

    modules_device_init(&device->base, RTC_DEVICE_TYPE, "rtc");

    device->get_value = get_value;

}

