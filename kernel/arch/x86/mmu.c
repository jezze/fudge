#include <lib/memory.h>
#include <kernel/error.h>
#include <kernel/log.h>
#include <kernel/arch/x86/cpu.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/mmu.h>

static struct mmu_directory mmuKernelDirectory;
static struct mmu_table mmuKernelTable;
static struct mmu_header mmuProgramHeaders[8];
unsigned int mmuProgramHeadersCount;
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

static void mmu_clear_directory(struct mmu_directory *directory)
{

    memory_set(directory, 0, sizeof (struct mmu_directory));

}

static void mmu_clear_table(struct mmu_table *table)
{

    memory_set(table, 0, sizeof (struct mmu_table));

}

static struct mmu_table *mmu_get_table(struct mmu_directory *directory, unsigned int frame)
{

    return (struct mmu_table *)((unsigned int)directory->tables[frame / MMU_DIRECTORY_SIZE] & 0xFFFFF000);

}

static unsigned int *mmu_get_entry(struct mmu_directory *directory, unsigned int frame)
{

    return &mmu_get_table(directory, frame)->entries[frame % MMU_TABLE_SIZE];

}

void mmu_map(struct mmu_directory *directory, struct mmu_table *table, void *virtualAddress, void *physicalAddress, unsigned int size, unsigned int tableFlags, unsigned int pageFlags)
{

    unsigned int frame = (unsigned int)virtualAddress / MMU_PAGE_SIZE;
    unsigned int index = frame / MMU_DIRECTORY_SIZE;
    unsigned int count = size / MMU_PAGE_SIZE + ((size & 0xFFF) > 0);

    mmu_clear_table(table);
    directory->tables[index] = (struct mmu_table *)((unsigned int)table | tableFlags);

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        *mmu_get_entry(directory, frame + i) = (unsigned int)physicalAddress | pageFlags;

        physicalAddress += MMU_PAGE_SIZE;

    }

}

void mmu_map_header(struct mmu_header *header, void *virtualAddress, void *physicalAddress, unsigned int size, unsigned int tableFlags, unsigned int pageFlags)
{

    unsigned int frame = (unsigned int)virtualAddress / MMU_PAGE_SIZE;
    unsigned int index = frame / MMU_DIRECTORY_SIZE;
    unsigned int count = size / MMU_PAGE_SIZE + ((size & 0xFFF) > 0);

    mmu_clear_table(&header->programTable);
    header->directory.tables[index] = (struct mmu_table *)((unsigned int)&header->programTable | tableFlags);

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        *mmu_get_entry(&header->directory, frame + i) = (unsigned int)physicalAddress | pageFlags;

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

struct mmu_header *mmu_get_program_header(void *address)
{

    unsigned int index = (unsigned int)address - 0x00380000;
    index = index / 0x00010000;

    return &mmuProgramHeaders[index];

}

void *mmu_get_slot()
{

    void *slot = (void *)(0x00380000 + mmuSlotCount * 0x00010000);

    mmuSlotCount++;

    if (mmuSlotCount == mmuProgramHeadersCount)
        mmuSlotCount = 0;

    return slot;

}

void mmu_init()
{

    mmuSlotCount = 0;
    mmuProgramHeadersCount = 8;

    mmu_clear_directory(&mmuKernelDirectory);
    mmu_map(&mmuKernelDirectory, &mmuKernelTable, (void *)0x00000000, (void *)0x00000000, 0x00400000, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);
    mmu_set_directory(&mmuKernelDirectory);

    unsigned int i;

    for (i = 0; i < mmuProgramHeadersCount; i++)
    {

        memory_copy(&mmuProgramHeaders[i].directory, &mmuKernelDirectory, sizeof (struct mmu_directory));

        struct mmu_table *table = mmu_get_table(&mmuProgramHeaders[i].directory, 0);
        table = &mmuKernelTable;

    }

    isr_register_handler(ISR_ROUTINE_PF, mmu_handler);

    mmu_enable();

}

