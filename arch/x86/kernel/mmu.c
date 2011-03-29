#include <lib/memory.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <arch/x86/kernel/cpu.h>
#include <arch/x86/kernel/isr.h>
#include <arch/x86/kernel/mmu.h>

struct mmu_directory mmuKernelDirectory;
struct mmu_table mmuKernelTable;
struct mmu_table mmuProgramTable;

void mmu_handler(struct isr_registers *registers)
{

    unsigned int address = cpu_get_cr2();

    log_message(LOG_TYPE_ERROR, "PAGE FAULT", 0);

    if (!(registers->error & MMU_ERROR_PRESENT))
        log_message(LOG_TYPE_ERROR, "present", 0);

    if (registers->error & MMU_ERROR_RW)
        log_message(LOG_TYPE_ERROR, "read-only", 0);

    if (registers->error & MMU_ERROR_USER)
        log_message(LOG_TYPE_ERROR, "user-mode", 0);

    if (registers->error & MMU_ERROR_RESERVED)
        log_message(LOG_TYPE_ERROR, "reserved", 0);

    if (registers->error & MMU_ERROR_FETCH)
        log_message(LOG_TYPE_ERROR, "fetch", 0);

    void *args[] = {&address};
    log_message(LOG_TYPE_ERROR, "Address: 0x%x", args);

    kernel_panic("PAGE FAULT", __FILE__, __LINE__);

}

static struct mmu_table *mmu_get_table(struct mmu_directory *directory, unsigned int frame)
{

    return (struct mmu_table *)((unsigned int)directory->tables[frame / MMU_DIRECTORY_SIZE] & 0xFFFFF000);

}

static unsigned int *mmu_get_entry(struct mmu_directory *directory, unsigned int frame)
{

    return &mmu_get_table(directory, frame)->entries[frame % MMU_TABLE_SIZE];

}

void mmu_map(struct mmu_directory *directory, unsigned int virtualAddress, unsigned int physicalAddress, unsigned int size, unsigned int flags)
{

    unsigned int frame = virtualAddress / MMU_PAGE_SIZE;
    unsigned int count = (size % MMU_PAGE_SIZE) ? (size / MMU_PAGE_SIZE) + MMU_PAGE_SIZE : (size / MMU_PAGE_SIZE);
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        *mmu_get_entry(directory, frame + i) = physicalAddress | flags;

        physicalAddress += MMU_PAGE_SIZE;

    }

}

void mmu_set_directory(struct mmu_directory *directory)
{

    cpu_set_cr3((unsigned int)directory);

}

void mmu_enable()
{

    cpu_set_cr0(cpu_get_cr0() | 0x80000000);

}

struct mmu_directory *mmu_get_kernel_directory()
{

    return &mmuKernelDirectory;

}

struct mmu_table *mmu_get_kernel_table()
{

    return &mmuKernelTable;

}

struct mmu_table *mmu_get_program_table()
{

    return &mmuProgramTable;

}

void mmu_clear_directory(struct mmu_directory *directory)
{

    memory_set(directory, 0, sizeof (struct mmu_directory));

}

void mmu_clear_table(struct mmu_table *table)
{

    memory_set(table, 0, sizeof (struct mmu_table));

}

void mmu_add_table(struct mmu_directory *directory, unsigned int index, struct mmu_table *table, unsigned int flags)
{

    directory->tables[index] = (struct mmu_table *)((unsigned int)table | flags);

}

static void mmu_init_kernel_directory()
{

    mmu_clear_directory(&mmuKernelDirectory);
    mmu_clear_table(&mmuKernelTable);
    mmu_add_table(&mmuKernelDirectory, 0, &mmuKernelTable, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE);
    mmu_map(&mmuKernelDirectory, 0x00000000, 0x00000000, 0x00400000, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);

}

void mmu_init()
{

    mmu_init_kernel_directory();
    mmu_set_directory(&mmuKernelDirectory);

    isr_register_handler(ISR_ROUTINE_PF, mmu_handler);

    mmu_enable();

}

