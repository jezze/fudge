#include <lib/vfs.h>
#include <kernel/arch/i386/arch.h>
#include <kernel/arch/i386/io.h>
#include <kernel/arch/i386/rtc.h>

unsigned char rtc_read(unsigned char type)
{

    outb(RTC_PORT_WRITE, type);
    return inb(RTC_PORT_READ);

}

void rtc_ready()
{

//    do { outb(RTC_PORT_WRITE, 0x0A); }
//    while (inb(RTC_PORT_READ) != 0x80);

}

void rtc_init()
{

    rtc_ready();

    arch_puts("Date: 20");
    arch_puts_bcd(rtc_read(RTC_FLAG_YEAR));
    arch_puts("-");
    arch_puts_bcd(rtc_read(RTC_FLAG_MONTH));
    arch_puts("-");
    arch_puts_bcd(rtc_read(RTC_FLAG_DAY));
    arch_puts(" ");
    arch_puts_bcd(rtc_read(RTC_FLAG_HOURS) & 0x0F);
    arch_puts(":");
    arch_puts_bcd(rtc_read(RTC_FLAG_MINUTES));
    arch_puts(":");
    arch_puts_bcd(rtc_read(RTC_FLAG_SECONDS));

    if (rtc_read(RTC_FLAG_HOURS) >> 4)
        arch_puts("PM\n");
    else
        arch_puts("AM\n");

}

