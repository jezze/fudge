#ifndef KERNEL_RUNTIME_H
#define KERNEL_RUNTIME_H

struct runtime_task
{

    struct mmu_header *header;

};

extern void runtime_init();

#endif

