#define MULTI_TASK_SLOTS                8

struct multi_task
{

    struct runtime_task base;
    struct mmu_directory *directory;
    struct mmu_table *ktable;
    struct mmu_table *utable;

};

struct multi_spawn_args
{

    void *caller;
    unsigned int index;

};
