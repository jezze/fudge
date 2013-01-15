#include <fudge/kernel.h>
#include <kernel/error.h>
#include <kernel/runtime.h>
#include "arch.h"
#include "cpu.h"
#include "idt.h"
#include "mmu.h"

static struct mmu_directory directory;
static struct mmu_table tables[3];

static void enable()
{

    cpu_set_cr0(cpu_get_cr0() | 0x80000000);

}

static void set_directory_table(struct mmu_directory *directory, unsigned int frame, struct mmu_table *table, unsigned int tflags)
{

    directory->tables[frame / MMU_TABLE_SLOTS] = (struct mmu_table *)((unsigned int)table | tflags);

}

static void set_table_page(struct mmu_table *table, unsigned int frame, unsigned int page, unsigned int pflags)
{

    table->pages[frame % MMU_PAGE_SLOTS] = (void *)(page | pflags);

}

void mmu_map_memory(struct mmu_directory *directory, struct mmu_table *table, unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    unsigned int frame = vaddress / MMU_PAGE_SIZE;
    unsigned int i;

    memory_clear(table, sizeof (struct mmu_table));

    for (i = 0; i < size / MMU_PAGE_SIZE; i++)
        set_table_page(table, frame + i, paddress + i * MMU_PAGE_SIZE, pflags);

    set_directory_table(directory, frame, table, tflags);

}

void mmu_load_memory(struct mmu_directory *directory)
{

    cpu_set_cr3((unsigned int)directory);

}

void mmu_reload_memory()
{

    cpu_set_cr3(cpu_get_cr3());

}

void mmu_interrupt(struct mmu_registers *registers)
{

    unsigned int address = cpu_get_cr2();

    error_register(1, address);
    error_register(2, registers->type);

}

void mmu_setup_arch(unsigned short selector)
{

    idt_set_entry(IDT_INDEX_PF, mmu_routine, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    mmu_map_memory(&directory, &tables[0], ARCH_KERNEL_BASE, ARCH_KERNEL_BASE, ARCH_KERNEL_SIZE, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);
    mmu_map_memory(&directory, &tables[1], RUNTIME_TASKADDRESS_PHYSICAL, RUNTIME_TASKADDRESS_VIRTUAL, RUNTIME_TASKADDRESS_SIZE, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);
    mmu_map_memory(&directory, &tables[2], RUNTIME_STACKADDRESS_PHYSICAL, RUNTIME_STACKADDRESS_VIRTUAL - RUNTIME_STACKADDRESS_SIZE, RUNTIME_STACKADDRESS_SIZE, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);
    mmu_load_memory(&directory);
    enable();

}

