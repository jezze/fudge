#define MULTI_TASK_SLOTS                32

struct multi_task
{

    struct mmu_directory directory;
    struct mmu_table ktable;
    struct mmu_table utable;
    struct runtime_task base;

};

struct multi_spawn_args
{

    void *caller;
    unsigned int index;

};
