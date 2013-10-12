#define PIT_FREQUENCY                   1193182
#define PIT_HERTZ                       100

struct pit_driver
{

    struct base_driver base;
    struct base_timer_interface itimer;
    struct kernel_rendezvous rduration;
    unsigned int duration;
    unsigned short divisor;

};

void pit_init_driver(struct pit_driver *driver);
