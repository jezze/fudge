struct rtc_driver
{

    struct base_driver base;
    struct base_clock_interface iclock;

};

void rtc_init_driver(struct rtc_driver *driver);
