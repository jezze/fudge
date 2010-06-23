#include <lib/types.h>
#include <lib/io.h>
#include <kernel/screen.h>
#include <kernel/rtc.h>

uint8_t rtc_read(uint8_t type)
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

    screen_puts("Date: 20");
    screen_puts_bcd(rtc_read(RTC_FLAG_YEAR));
    screen_puts("-");
    screen_puts_bcd(rtc_read(RTC_FLAG_MONTH));
    screen_puts("-");
    screen_puts_bcd(rtc_read(RTC_FLAG_DAY));
    screen_puts(" ");
    screen_puts_bcd(rtc_read(RTC_FLAG_HOURS));
    screen_puts(":");
    screen_puts_bcd(rtc_read(RTC_FLAG_MINUTES));
    screen_puts(":");
    screen_puts_bcd(rtc_read(RTC_FLAG_SECONDS));
    screen_puts("\n");

}

