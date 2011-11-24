#include <lib/memory.h>
#include <kernel/error.h>
#include <kernel/mmu.h>
#include <kernel/log.h>
#include <kernel/arch/x86/cpu.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/mmu.h>

static struct mmu_unit mmuUnit;
static struct mmu_header mmuKernelHeader;
static struct mmu_header mmuProgramHeaders[8];

static void handle_isr(struct isr_registers *registers)
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

static void clear_directory(struct mmu_directory *directory)
{

    memory_set(directory, 0, sizeof (struct mmu_directory));

}

static void clear_table(struct mmu_table *table)
{

    memory_set(table, 0, sizeof (struct mmu_table));

}

static struct mmu_table *get_table(struct mmu_directory *directory, unsigned int frame)
{

    return (struct mmu_table *)((unsigned int)directory->tables[frame / MMU_DIRECTORY_SLOTS] & 0xFFFFF000);

}

static unsigned int *get_entry(struct mmu_directory *directory, unsigned int frame)
{

    return &get_table(directory, frame)->entries[frame % MMU_TABLE_SLOTS];

}

static struct mmu_header *get_header(void *paddress)
{

    if (!paddress)
        return &mmuKernelHeader;

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        if (mmuProgramHeaders[i].address == paddress)
            return &mmuProgramHeaders[i];

    }

    return 0;

}

static void mmu_unit_map(void *paddress, void *vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    struct mmu_header *header = get_header(paddress);

    unsigned int frame = (unsigned int)vaddress / MMU_PAGE_SLOTS;
    unsigned int index = frame / MMU_DIRECTORY_SLOTS;
    unsigned int count = size / MMU_PAGE_SIZE + ((size & 0xFFF) > 0);

    clear_table(&header->table);
    header->directory.tables[index] = (struct mmu_table *)((unsigned int)&header->table | tflags);

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        *get_entry(&header->directory, frame + i) = (unsigned int)paddress | pflags;

        paddress += MMU_PAGE_SIZE;

    }

}

static void mmu_unit_set_directory(void *paddress)
{

    struct mmu_header *header = get_header(paddress);

    cpu_set_cr3((unsigned int)&header->directory);

}

static void mmu_unit_enable()
{

    cpu_set_cr0(cpu_get_cr0() | 0x80000000);

}

static void *mmu_unit_get_paddress(unsigned int id)
{

    return mmuProgramHeaders[id].address;

}

static void mmu_unit_setup()
{

    clear_directory(&mmuKernelHeader.directory);
    mmuKernelHeader.address = 0;
    mmu_unit_map(mmuKernelHeader.address, (void *)0x00000000, 0x00400000, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);
    mmu_unit_set_directory(mmuKernelHeader.address);

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        mmuProgramHeaders[i].address = (void *)(0x00300000 + i * 0x10000);
        memory_copy(&mmuProgramHeaders[i].directory, &mmuKernelHeader.directory, sizeof (struct mmu_directory));

    }

    isr_register_handler(ISR_ROUTINE_PF, handle_isr);

}

void mmu_init()
{

    mmu_unit_init(&mmuUnit, mmu_unit_setup, mmu_unit_enable, mmu_unit_get_paddress, mmu_unit_set_directory, mmu_unit_map);
    mmu_register_unit(&mmuUnit);

}

