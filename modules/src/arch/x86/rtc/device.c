#include <memory.h>
#include <modules/modules.h>
#include <arch/x86/io/io.h>
#include <arch/x86/rtc/rtc.h>

static unsigned char convert(unsigned char num)
{

    return ((num & 0xF0) >> 1) + ((num & 0xF0) >> 3) + (num & 0x0F);

}

static unsigned char get_value(unsigned int type)
{

    io_outb(RTC_PORT_WRITE, type);

    return convert(io_inb(RTC_PORT_READ));

}

void rtc_device_init(struct rtc_device *device, unsigned int irq)
{

    memory_clear(device, sizeof (struct rtc_device));

    modules_device_init(&device->base, RTC_DEVICE_TYPE, "rtc");

    device->irq = irq;
    device->get_value = get_value;

}

