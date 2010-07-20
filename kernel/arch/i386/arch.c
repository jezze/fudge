#include <lib/cbuffer.h>
#include <lib/vfs.h>
#include <kernel/arch/i386/arch.h>
#include <kernel/arch/i386/gdt.h>
#include <kernel/arch/i386/idt.h>
#include <kernel/arch/i386/isr.h>
#include <kernel/arch/i386/irq.h>
#include <kernel/arch/i386/kbd.h>
#include <kernel/arch/i386/mmu.h>
#include <kernel/arch/i386/pit.h>
#include <kernel/arch/i386/screen.h>
#include <kernel/arch/i386/syscall.h>
#include <kernel/assert.h>

void arch_clear()
{

    screen_clear();

}

void arch_putc(char c)
{

    screen_putc(c);

}

void arch_puts(char *s)
{

    unsigned int i;

    for (i = 0; s[i] != '\0'; i++)
        arch_putc(s[i]);

}

void arch_puts_num(unsigned int n, unsigned int base)
{

    if (!n)
    {

        arch_putc('0');
        return;

    }

    char s[32] = {0};

    int i;

    for (i = 30; n && i; --i, n /= base)
        s[i] = "0123456789abcdef"[n % base];

    arch_puts(s + i + 1);

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

struct vfs_node *arch_get_stdin()
{

    return kbd_get_node();

}

struct vfs_node *arch_get_stdout()
{

    return screen_get_node();

}

void arch_disable_interrupts()
{

    isr_disable();

}

void arch_enable_interrupts()
{

    isr_enable();

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

}

