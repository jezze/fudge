#ifndef KERNEL_MULTI_H
#define KERNEL_MULTI_H

#define MULTI_TASK_SLOTS 32

struct runtime_task *multi_get_task(unsigned int index);
unsigned int multi_get_task_slot(unsigned int parent);
void multi_clone_task(struct runtime_task *task, struct runtime_task *from, unsigned int id, unsigned int ip);
struct runtime_task *multi_schedule();
struct runtime_task *multi_setup();

#endif

