#include <lib/memory.h>
#include <kernel/error.h>
#include <kernel/log.h>
#include <kernel/arch/x86/cpu.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/mmu.h>

static struct mmu_table mmuUserTable;
static struct mmu_table mmuProgramTable;

static struct mmu_header mmuKernelHeader;
static struct mmu_header mmuProgramHeaders[16];

unsigned int mmuSlotCount;

static void mmu_handler(struct isr_registers *registers)
{

    unsigned int address = cpu_get_cr2();

    log_write("ERROR!\n");

    if (!(registers->error & MMU_ERROR_PRESENT))
        log_write("Page: present\n");

    if (registers->error & MMU_ERROR_RW)
        log_write("Page: read-only\n");

    if (registers->error & MMU_ERROR_USER)
        log_write("Page: user-mode\n");

    if (registers->error & MMU_ERROR_RESERVED)
        log_write("Page: reserved\n");

    if (registers->error & MMU_ERROR_FETCH)
        log_write("Page: fetch\n");

    log_write("Address: 0x%x\n", address);

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

    return &mmuKernelHeader.directory;

}

struct mmu_table *mmu_get_kernel_table()
{

    return &mmuKernelHeader.table;

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

void *mmu_get_slot()
{

    void *slot = (void *)(0x00400000 + mmuSlotCount * 0x00100000);

    mmuSlotCount++;

    if (mmuSlotCount == 3)
        mmuSlotCount = 0;

    return slot;

}

static void mmu_init_kernel_directory()
{

    mmu_clear_directory(&mmuKernelHeader.directory);

    mmu_clear_table(&mmuKernelHeader.table);
    mmu_add_table(&mmuKernelHeader.directory, 0, &mmuKernelHeader.table, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE);
    mmu_map(&mmuKernelHeader.directory, 0x00000000, 0x00000000, 0x00400000, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);

    mmu_clear_table(&mmuUserTable);
    mmu_add_table(&mmuKernelHeader.directory, 1, &mmuUserTable, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE);
    mmu_map(&mmuKernelHeader.directory, 0x00400000, 0x00400000, 0x00400000, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);

}

void mmu_init()
{

    mmuSlotCount = 0;

    mmu_init_kernel_directory();
    mmu_set_directory(&mmuKernelHeader.directory);

    isr_register_handler(ISR_ROUTINE_PF, mmu_handler);

    mmu_enable();

}

