#ifndef KERNEL_EVENT_H
#define KERNEL_EVENT_H

#define EVENT_ROUTINE_OPEN    0x01
#define EVENT_ROUTINE_CLOSE   0x02
#define EVENT_ROUTINE_EXECUTE 0x10

struct event_event
{

    unsigned int id;
    void (*handler)();

};

extern void event_register(unsigned int index, unsigned int id, void (*handler)());
extern void event_unregister(unsigned int index, unsigned int id);
extern struct event_event *event_get(unsigned int index);
extern unsigned int event_handler(unsigned int index);
extern void event_init();

#endif

