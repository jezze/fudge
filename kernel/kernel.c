#include <kernel/kernel.h>
#include <kernel/shell.h>
#include <kernel/tty.h>

void kernel_init()
{

    tty_init();
    shell_init();

    for (;;);

}

