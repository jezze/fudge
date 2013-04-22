#define RTC_FLAG_SECONDS                0x00
#define RTC_FLAG_MINUTES                0x02
#define RTC_FLAG_HOURS                  0x04
#define RTC_FLAG_DAY                    0x07
#define RTC_FLAG_MONTH                  0x08
#define RTC_FLAG_YEAR                   0x09

#define RTC_IRQ                         0x08

#define RTC_DEVICE_TYPE                 0x2000

struct rtc_device
{

    struct base_device base;
    unsigned char (*get_value)(unsigned int type);

};

void rtc_init_device(struct rtc_device *device, unsigned int irq);
