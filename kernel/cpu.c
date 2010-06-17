#include <lib/types.h>
#include <lib/string.h>
#include <kernel/screen.h>
#include <kernel/cpu.h>

void cpu_init()
{

    if (cpu_check_cpuid())
        screen_puts("CPU\n");

    char vendor[12];

    cpu_get_vendor(vendor);

    screen_puts("Processor: ");
    screen_putc(vendor[0]);
    screen_putc(vendor[1]);
    screen_putc(vendor[2]);
    screen_putc(vendor[3]);
    screen_putc(vendor[4]);
    screen_putc(vendor[5]);
    screen_putc(vendor[6]);
    screen_putc(vendor[7]);
    screen_putc(vendor[8]);
    screen_putc(vendor[9]);
    screen_putc(vendor[10]);
    screen_putc(vendor[11]);
    screen_putc('\n');

    screen_puts("\n");

}

