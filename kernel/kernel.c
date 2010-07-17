#include <lib/types.h>
#include <lib/cbuffer.h>
#include <lib/io.h>
#include <lib/vfs.h>
#include <lib/call.h>
#include <kernel/assert.h>
#include <kernel/screen.h>
#include <kernel/mboot.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/isr.h>
#include <kernel/irq.h>
#include <kernel/pit.h>
#include <kernel/kbd.h>
#include <kernel/mmu.h>
#include <kernel/initrd.h>
#include <kernel/shell.h>
#include <kernel/syscall.h>
#include <kernel/rtc.h>
#include <kernel/kernel.h>

struct kernel kernel;
struct vfs_node *fsRoot = 0;

unsigned int kernel_reboot()
{

    isr_disable();

    unsigned char ready = 0x02;

    while ((ready & 0x02) != 0)
        ready = inb(0x64);

    outb(0x64, 0xFE);

    return 0;

}

static void kernel_init_shell()
{

    struct vfs_node *node = call_vfs_find("shell");

    char *buffer = (char *)0x1F0000;

    vfs_read(node, 0, 2000, buffer);
        
    void (*func)(int argc, char *argv[]) = (void (*)(int argc, char *argv[]))0x1F0000;

    func(0, 0);

}

void kernel_main(struct mboot_info *header, uint32_t magic)
{

    gdt_init();
    idt_init();

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

