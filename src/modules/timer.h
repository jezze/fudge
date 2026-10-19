#define TIMER_SLOTS                     64
#define TIMER_ENTRIES                   64

struct timer_entry
{

    struct list_item item;
    unsigned int target;
    unsigned int id;
    unsigned int expires;
    unsigned int period;

};

struct timer_interface
{

    struct resource resource;
    struct service service;
    unsigned int id;
    unsigned int hz;
    unsigned int jiffies;
    struct spinlock spinlock;
    struct list slots[TIMER_SLOTS];
    struct list free;
    struct timer_entry entries[TIMER_ENTRIES];

};

void timer_notifytick(struct timer_interface *interface, unsigned int counter);
void timer_registerinterface(struct timer_interface *interface);
void timer_unregisterinterface(struct timer_interface *interface);
void timer_initinterface(struct timer_interface *interface, unsigned int id, unsigned int hz);
