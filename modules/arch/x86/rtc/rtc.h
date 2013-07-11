#define RTC_IRQ                         0x08
#define RTC_DEVICE_TYPE                 0x2000

struct rtc_device
{

    struct base_device base;
    unsigned char (*get_value)(unsigned int type);

};

void rtc_init_device(struct rtc_device *device, unsigned int irq);
