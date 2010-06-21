#include <lib/types.h>
#include <kernel/screen.h>
#include <kernel/mboot.h>

void mboot_init(mboot_info_t *header)
{

    if (header->flags & MBOOT_FLAG_MEM)
    {

        screen_puts("MEMORY\n");

        screen_puts("Lower memory: ");
        screen_puts_dec(header->memoryLower);
        screen_puts("KB\n");

        screen_puts("Upper memory: ");
        screen_puts_dec(header->memoryUpper);
        screen_puts("KB\n");

        screen_puts("\n");

    }

    if (header->flags & MBOOT_FLAG_MMAP)
    {

    }

    if (header->flags & MBOOT_FLAG_VBE)
    {


    }

}
