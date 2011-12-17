#include <kernel/mmu.h>

static struct mmu_unit *primary;

void *mmu_get_task_memory(unsigned int id)
{

    return primary->get_task_memory(id);

}

void mmu_load_task_memory(void *paddress)
{

    primary->load_task_memory(paddress);

}

void mmu_map_task_memory(void *paddress, void *vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    primary->map_task_memory(paddress, vaddress, size, tflags, pflags);

}

void mmu_unit_init(struct mmu_unit *unit, void (*setup)(), void (*enable)(), void *(*get_task_memory)(unsigned int id), void (*load_task_memory)(void *paddress), void (*map_task_memory)(void *paddress, void *vaddress, unsigned int size, unsigned int tflags, unsigned int pflags))
{

    unit->setup = setup;
    unit->enable = enable;
    unit->get_task_memory = get_task_memory;
    unit->load_task_memory = load_task_memory;
    unit->map_task_memory = map_task_memory;

    unit->setup();
    unit->enable();

}

void mmu_register_unit(struct mmu_unit *unit)
{

    primary = unit;

}

