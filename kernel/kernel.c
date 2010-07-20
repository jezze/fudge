#include <lib/cbuffer.h>
#include <lib/vfs.h>
#include <lib/call.h>
#include <kernel/arch/i386/arch.h>
#include <kernel/arch/i386/io.h>
#include <kernel/arch/i386/screen.h>
#include <kernel/arch/i386/isr.h>
#include <kernel/arch/i386/irq.h>
#include <kernel/arch/i386/pit.h>
#include <kernel/arch/i386/kbd.h>
#include <kernel/arch/i386/mmu.h>
#include <kernel/arch/i386/initrd.h>
#include <kernel/arch/i386/syscall.h>
#include <kernel/arch/i386/rtc.h>
#include <kernel/assert.h>
#include <kernel/mboot.h>
#include <kernel/shell.h>
#include <kernel/kernel.h>

struct kernel kernel;
struct vfs_node *fsRoot = 0;

static void kernel_init_shell()
{

    struct vfs_node *node = call_vfs_find("shell");

    char *buffer = (char *)0x1F0000;

    vfs_read(node, 0, 2000, buffer);
        
    void (*func)(int argc, char *argv[]) = (void (*)(int argc, char *argv[]))0x1F0000;

    func(0, 0);

}

void kernel_main(struct mboot_info *header, unsigned int magic)
{

    arch_init();
    screen_init();

    ASSERT(magic == MBOOT_MAGIC);
    ASSERT(header->modulesCount);

    isr_init();
    isr_register_handler(0x0E, mmu_handler);

    irq_init();
    irq_register_handler(0x00, pit_handler);
    irq_register_handler(0x01, kbd_handler);

    syscall_init();

    mmu_init();
    
    pit_init();
    kbd_init();

    isr_enable();

    fsRoot = initrd_init(*((unsigned int *)header->modulesAddresses));

    mboot_init(header);

    shell_init();

    for (;;);

}

