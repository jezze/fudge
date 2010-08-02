#include <lib/call.h>
#include <lib/cbuffer.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/arch/x86/arch.h>
#include <kernel/arch/x86/gdt.h>
#include <kernel/arch/x86/idt.h>
#include <kernel/arch/x86/io.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/irq.h>
#include <kernel/arch/x86/kbd.h>
#include <kernel/arch/x86/mmu.h>
#include <kernel/arch/x86/pit.h>
#include <kernel/arch/x86/screen.h>
#include <kernel/arch/x86/syscall.h>
#include <kernel/assert.h>

struct vfs_node *stdin;
struct vfs_node *stdout;

char arch_getc()
{

    struct vfs_node *in = stdin;

    char c;

    while (!(vfs_read(in, 0, 1, &c)));

    return c;

}

void arch_clear()
{

    screen_clear();

}

void arch_puts_num(unsigned int n, unsigned int base)
{

    if (!n)
    {

        call_putc('0');
        return;

    }

    char s[32] = {0};

    int i;

    for (i = 30; n && i; --i, n /= base)
        s[i] = "0123456789abcdef"[n % base];

    call_puts(s + i + 1);

}

void arch_puts_dec(unsigned int n)
{

    arch_puts_num(n, 10);

}

void arch_puts_hex(unsigned int n)
{

    arch_puts_num(n, 16);

}

void arch_puts_bcd(unsigned char n)
{

    arch_puts_dec(n >> 4);
    arch_puts_dec(n & 0x0F);

}

void arch_set_stdin(struct vfs_node *node)
{

    stdin = node;

    kbd_set_node(node);

}

void arch_set_stdout(struct vfs_node *node)
{

    stdout = node;

    screen_set_node(node);

}

void arch_disable_interrupts()
{

    isr_disable();

}

void arch_enable_interrupts()
{

    isr_enable();

}

void arch_reboot()
{

    isr_disable();

    unsigned char ready = 0x02;

    while ((ready & 0x02) != 0)
        ready = io_inb(0x64);

    io_outb(0x64, 0xFE);

}

void arch_init()
{

    gdt_init();
    idt_init();
    screen_init();
    mmu_init();

}

void arch_init_devices()
{

    pit_init();
    kbd_init();

}

void arch_init_interrupts()
{

    isr_init();
    isr_register_handler(0x0E, mmu_handler);

    irq_init();
    irq_register_handler(0x00, pit_handler);
    irq_register_handler(0x01, kbd_handler);

}

void arch_init_syscalls()
{

    syscall_init();
    syscall_register_handler(CALL_VFS_WALK, syscall_vfs_walk);
    syscall_register_handler(CALL_VFS_FIND, syscall_vfs_find);
    syscall_register_handler(CALL_REBOOT, syscall_reboot);

}

