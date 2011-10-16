#include <lib/elf.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/runtime.h>
#include <kernel/arch/x86/mmu.h>
#include <modules/elf/elf.h>

static struct kernel kernel;

void kernel_disable_interrupts()
{

    kernel.arch->disable_interrupts();

}

void kernel_enable_interrupts()
{

    kernel.arch->enable_interrupts();

}

void kernel_register_irq(unsigned int index, void (*handler)())
{

    kernel.arch->register_irq(index, handler);

}

void kernel_unregister_irq(unsigned int index)
{

    kernel.arch->unregister_irq(index);

}

static void kernel_init_shell()
{

    struct runtime_task *task = runtime_get_running_task();
    task->load(task, "/shell", 0, 0);

    struct mmu_header *pHeader = mmu_get_program_header();
    struct elf_header *header = elf_get_header(pHeader->address);
    struct elf_program_header *programHeader = elf_get_program_header(header);

    mmu_map(pHeader, programHeader->virtualAddress, 0x10000, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);
    mmu_set_directory(&pHeader->directory);

    struct vfs_node *sin = vfs_find("/tty");
    struct vfs_node *sout = vfs_find("/tty");
    struct vfs_node *serror = vfs_find("/serial");

    task->add_descriptor(task, sin);
    task->add_descriptor(task, sout);
    task->add_descriptor(task, serror);

    kernel.arch->enter_usermode(header->entry, programHeader->virtualAddress + 0xFFF4);

}

void kernel_init(struct kernel_arch *arch)
{

    log_init();

    kernel.arch = arch;
    kernel.arch->setup(kernel.arch);
    kernel.arch->enable_interrupts();

    vfs_init();
    initrd_init(kernel.arch->initrdAddress);
    modules_init();
    runtime_init();

    kernel.arch->set_stack(arch->stackAddress);
    kernel_init_shell();

    for (;;);

}

