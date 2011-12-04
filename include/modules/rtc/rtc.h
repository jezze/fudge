#ifndef MODULES_RTC_H
#define MODULES_RTC_H

#define RTC_PORT_WRITE 0x70
#define RTC_PORT_READ  0x71

#define RTC_FLAG_SECONDS 0x00
#define RTC_FLAG_MINUTES 0x02
#define RTC_FLAG_HOURS   0x04
#define RTC_FLAG_DAY     0x07
#define RTC_FLAG_MONTH   0x08
#define RTC_FLAG_YEAR    0x09

#define RTC_DEVICE_TYPE 2000

struct rtc_device
{

    struct modules_device base;
    unsigned char (*get)(unsigned int type);
    struct vfs_view view;
    struct vfs_node read;

};

extern void rtc_device_init(struct rtc_device *device);

#endif

