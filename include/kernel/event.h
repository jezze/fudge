#ifndef KERNEL_EVENT_H
#define KERNEL_EVENT_H

#define EVENT_ROUTINE_OPEN    0x01
#define EVENT_ROUTINE_CLOSE   0x02
#define EVENT_ROUTINE_EXECUTE 0x10

extern void event_register(unsigned int index, void (*handler)());
extern void event_unregister(unsigned int index);
extern void event_run(unsigned int index);
extern void event_init();

#endif

