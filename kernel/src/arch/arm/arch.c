#include <memory.h>
#include <string.h>
#include <kernel.h>
#include <arch/arm/arch.h>
#include <arch/arm/cpu.h>
#include <arch/arm/kmi.h>
#include <arch/arm/pic.h>
#include <arch/arm/reg.h>
#include <arch/arm/uart.h>

static void *ramdiskv[1];
static struct arch_interface interface;

/*
static void enable_interrupts()
{

    reg_write32(PIC_REG_IRQ_ENABLECLR, PIC_IRQ_KEYBOARD);

    reg_write32(KMI_REG_KBD_CONTROL, KMI_CONTROL_INTRX | KMI_CONTROL_INTTX);

    reg_write32(PIC_REG_IRQ_ENABLESET, reg_read32(PIC_REG_IRQ_ENABLESET) | PIC_IRQ_KEYBOARD);

}
*/

static void setup(struct kernel_interface *arch)
{

    uart_puts("Fudge Console Text\n");
    uart_puts("SETUP\n");

}

static void enter_usermode(unsigned int ip, unsigned int sp)
{

    char num[32];

    uart_puts("USERMODE\n");

    uart_puts("IP: ");
    string_write_num(num, ip, 16);
    uart_puts(num);
    uart_puts(" ");

    uart_puts("SP: ");
    string_write_num(num, sp, 16);
    uart_puts(num);
    uart_puts("\n");

    kmi_setup();

}

void isr_undefined()
{

    uart_puts("INTERRUPT\n");

    for (;;);

}

void arch_init_interface(struct arch_interface *interface)
{

    memory_clear(interface, sizeof (struct arch_interface));

    ramdiskv[0] = (void *)0x00800000;

    kernel_init_interface(&interface->base, setup, enter_usermode, 1, ramdiskv);

}

void arch_setup()
{

    arch_init_interface(&interface);

    kernel_register_interface(&interface.base);

}

