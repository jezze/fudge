struct pit_driver
{

    struct base_driver base;
    struct base_timer_interface itimer;
    struct rendezvous rduration;
    unsigned int duration;
    unsigned short divisor;

};

void pit_init_driver(struct pit_driver *driver);
