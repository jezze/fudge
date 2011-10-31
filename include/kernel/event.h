#ifndef KERNEL_EVENT_H
#define KERNEL_EVENT_H

extern void event_attach(unsigned int index, void (*handler)());
extern void event_run(unsigned int index);
extern void event_init();

#endif

