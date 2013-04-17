#define MULTI_TASK_BASE                 0x00300000
#define MULTI_TASK_SLOTS                32

struct multi_task
{

    struct mmu_directory directory;
    struct mmu_table tables[2];
    struct runtime_task base;

};
