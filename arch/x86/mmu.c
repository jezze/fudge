#include <lib/memory.h>
#include <kernel/error.h>
#include <kernel/log.h>
#include <arch/x86/cpu.h>
#include <arch/x86/isr.h>
#include <arch/x86/mmu.h>

static struct mmu_directory mmuKernelDirectory;
static struct mmu_table mmuKernelTable;
static struct mmu_table mmuUserTable;
static struct mmu_table mmuProgramTable;

static void mmu_handler(struct isr_registers *registers)
{

    unsigned int address = cpu_get_cr2();

    log_string("ERROR!\n");

    if (!(registers->error & MMU_ERROR_PRESENT))
        log_string("Page: present\n");

    if (registers->error & MMU_ERROR_RW)
        log_string("Page: read-only\n");

    if (registers->error & MMU_ERROR_USER)
        log_string("Page: user-mode\n");

    if (registers->error & MMU_ERROR_RESERVED)
        log_string("Page: reserved\n");

    if (registers->error & MMU_ERROR_FETCH)
        log_string("Page: fetch\n");

    log_string("Address: 0x");
    log_num(address, 16);
    log_string("\n");

    error_panic("PAGE FAULT", __FILE__, __LINE__);

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
    unsigned int count = size / MMU_PAGE_SIZE + ((size & 0xFFF) > 0);
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

    mmu_clear_table(&mmuUserTable);
    mmu_add_table(&mmuKernelDirectory, 1, &mmuUserTable, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE);
    mmu_map(&mmuKernelDirectory, 0x00400000, 0x00400000, 0x00400000, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);

}

void mmu_init()
{

    mmu_init_kernel_directory();
    mmu_set_directory(&mmuKernelDirectory);

    isr_register_handler(ISR_ROUTINE_PF, mmu_handler);

    mmu_enable();

}

