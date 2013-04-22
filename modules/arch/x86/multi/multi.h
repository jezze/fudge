struct multi_task
{

    struct mmu_directory directory;
    struct mmu_table tables[2];
    struct runtime_task base;

};
