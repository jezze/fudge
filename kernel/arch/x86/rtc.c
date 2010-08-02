#include <lib/call.h>
#include <lib/vfs.h>
#include <kernel/arch/x86/arch.h>
#include <kernel/arch/x86/io.h>
#include <kernel/arch/x86/rtc.h>

unsigned char rtc_read(unsigned char type)
{

    io_outb(RTC_PORT_WRITE, type);
    return io_inb(RTC_PORT_READ);

}

void rtc_ready()
{

//    do { io_outb(RTC_PORT_WRITE, 0x0A); }
//    while (io_inb(RTC_PORT_READ) != 0x80);

}

void rtc_init()
{

    rtc_ready();

    call_puts("Date: 20");
    call_puts_bcd(rtc_read(RTC_FLAG_YEAR));
    call_puts("-");
    call_puts_bcd(rtc_read(RTC_FLAG_MONTH));
    call_puts("-");
    call_puts_bcd(rtc_read(RTC_FLAG_DAY));
    call_puts(" ");
    call_puts_bcd(rtc_read(RTC_FLAG_HOURS) & 0x0F);
    call_puts(":");
    call_puts_bcd(rtc_read(RTC_FLAG_MINUTES));
    call_puts(":");
    call_puts_bcd(rtc_read(RTC_FLAG_SECONDS));

    if (rtc_read(RTC_FLAG_HOURS) >> 4)
        call_puts("PM\n");
    else
        call_puts("AM\n");

}

