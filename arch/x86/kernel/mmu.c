#include <lib/assert.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/session.h>
#include <lib/vfs.h>
#include <arch/x86/kernel/arch.h>
#include <arch/x86/kernel/cr.h>
#include <arch/x86/kernel/isr.h>
#include <arch/x86/kernel/mmu.h>

struct mmu_directory mmuDirectory;

void mmu_handler(struct isr_registers *registers)
{

    unsigned int address = cr2_read();

    file_write_string(session_get_out(), "PAGE FAULT (");

    if (!(registers->err_code & MMU_ERROR_PRESENT))
        file_write_string(session_get_out(), "present");

    if (registers->err_code & MMU_ERROR_RW)
        file_write_string(session_get_out(), "read-only");

    if (registers->err_code & MMU_ERROR_USER)
        file_write_string(session_get_out(), "user-mode");

    if (registers->err_code & MMU_ERROR_RESERVED)
        file_write_string(session_get_out(), "reserved");

    if (registers->err_code & MMU_ERROR_FETCH)
        file_write_string(session_get_out(), "fetch");

    file_write_string(session_get_out(), ") at 0x");
    file_write_hex(session_get_out(), address);
    file_write_string(session_get_out(), "\n");

    PANIC("PAGE FAULT");

}

static struct mmu_table *mmu_get_true_table_address(struct mmu_table *table)
{

    return (struct mmu_table *)((unsigned int)table & 0xFFFFF000);

}

static void mmu_map(unsigned int base, unsigned int limit, unsigned int directoryFlags, unsigned int tableFlags)
{

    unsigned int i;

    for (i = 0; base < limit; base += 0x1000, i++)
    {

        mmuDirectory.tables[i / 1024] = (struct mmu_table *)(MMU_TABLE_ADDRESS | directoryFlags);
        mmu_get_true_table_address(mmuDirectory.tables[i / 1024])->entries[i % 1024] = base | tableFlags;

    }

}

static void mmu_flush_directory(struct mmu_directory *directory)
{

    cr3_write((unsigned int)directory);
    cr0_write(cr0_read() | 0x80000000);

}

static void mmu_init_directory(struct mmu_directory *directory)
{

    memory_set(directory, 0, sizeof (struct mmu_directory));

}

void mmu_init()
{

    mmu_init_directory(&mmuDirectory);
    mmu_map(0x00000000, 0x00400000, MMU_DIRECTORY_FLAG_PRESENT | MMU_DIRECTORY_FLAG_WRITEABLE, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE);
//    mmu_map(0x00400000, 0x00800000, MMU_DIRECTORY_FLAG_PRESENT | MMU_DIRECTORY_FLAG_WRITEABLE | MMU_DIRECTORY_FLAG_USERMODE, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE);

    mmu_flush_directory(&mmuDirectory);

}

