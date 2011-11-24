#include <kernel/mmu.h>

static struct mmu_unit *mmuUnit;

void *mmu_get_task_memory(unsigned int id)
{

    return mmuUnit->get_task_memory(id);

}

void mmu_load_task_memory(void *paddress)
{

    mmuUnit->load_task_memory(paddress);

}

void mmu_map_task_memory(void *paddress, void *vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    mmuUnit->map_task_memory(paddress, vaddress, size, tflags, pflags);

}

void mmu_unit_init(struct mmu_unit *unit, void (*enable)(), void *(*get_task_memory)(unsigned int id), void (*load_task_memory)(void *paddress), void (*map_task_memory)(void *paddress, void *vaddress, unsigned int size, unsigned int tflags, unsigned int pflags))
{

    unit->enable = enable;
    unit->get_task_memory = get_task_memory;
    unit->load_task_memory = load_task_memory;
    unit->map_task_memory = map_task_memory;

}

void mmu_register_unit(struct mmu_unit *unit)
{

    mmuUnit = unit;

}

