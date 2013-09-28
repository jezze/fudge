#define RTC_IRQ                         0x08
#define RTC_DEVICE_TYPE                 0x2000

struct rtc_device
{

    struct base_device base;

};

struct rtc_driver
{

    struct base_driver base;
    struct base_clock_interface iclock;

};

void rtc_init_device(struct rtc_device *device, unsigned int irq);
void rtc_init_driver(struct rtc_driver *driver);
