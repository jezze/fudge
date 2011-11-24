#ifndef KERNEL_MMU_H
#define KERNEL_MMU_H

struct mmu_unit
{

    void *(*get_task_memory)(unsigned int id);
    void (*load_task_memory)(void *paddress);
    void (*map_task_memory)(void *paddress, void *vaddress, unsigned int size, unsigned int tflags, unsigned int pflags);

};

extern void *mmu_get_task_memory(unsigned int id);
extern void mmu_load_task_memory(void *paddress);
extern void mmu_map_task_memory(void *paddress, void *vaddress, unsigned int size, unsigned int tflags, unsigned int pflags);
extern void mmu_unit_init(struct mmu_unit *unit, void *(*get_task_memory)(unsigned int id), void (*load_task_memory)(void *paddress), void (*map_task_memory)(void *paddress, void *vaddress, unsigned int size, unsigned int tflags, unsigned int pflags));
extern void mmu_register_unit(struct mmu_unit *unit);

#endif

