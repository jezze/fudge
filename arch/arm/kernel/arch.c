#include <arch/arm/kernel/arch.h>

#define SERIAL_BASE 0x16000000
#define SERIAL_FLAG_REGISTER 0x18
#define SERIAL_BUFFER_FULL (1 << 5)

struct vfs_node *initrd_init(unsigned int location)
{

    location++;
    return 0;

}

void shell_init()
{

}

static void arch_putc(char c)
{

    while (*(volatile unsigned long*)(SERIAL_BASE + SERIAL_FLAG_REGISTER) & (SERIAL_BUFFER_FULL));

    *(volatile unsigned long*)SERIAL_BASE = c;

}

static void arch_puts(char *s)
{

    while (*s) arch_putc(*s++);

}

void arch_enable_interrupts()
{

}

void arch_disable_interrupts()
{

}

static void arch_put_pixel(volatile unsigned short *address, unsigned short value)
{

    *address = value;

}

void arch_init()
{

    arch_puts("Hello World!\n");

    volatile unsigned int *mem;

    mem = (volatile unsigned int *)0x10000014;
    *mem = 0xA05F;

    mem = (volatile unsigned int *)0x1000001C;
    *mem = 0x12C11;

    mem = (volatile unsigned int *)0xC0000000;
    *mem = 0x3F1F3F9C;

    mem = (volatile unsigned int *)0xC0000004;
    *mem = 0x080B61DF;

    mem = (volatile unsigned int *)0xC0000008;
    *mem = 0x067F3800;

    mem = (volatile unsigned int *)0xC0000010;
    *mem = 0x00200000;

    mem = (volatile unsigned int *)0xC000001C;
    *mem = 0x1829;

    mem = (volatile unsigned int *)0x1000000C;
    *mem = 0x3E005;

    volatile unsigned short *video;

    video = (volatile unsigned short *)0x00200000;
    *video = 0xFFFF;

    video = (volatile unsigned short *)0x00200002;
    *video = 0xFC00;

    video = (volatile unsigned short *)0x00200004;
    *video = 0x83E0;
    
    video = (volatile unsigned short *)0x00200006;
    *video = 0x801F;

    video = (volatile unsigned short *)0x00200008;

    arch_put_pixel(video, (unsigned short)0xFFFF);

}

void arch_init_devices()
{

}

void arch_init_interrupts()
{

}

void arch_init_syscalls()
{

}

