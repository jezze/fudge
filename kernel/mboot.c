#include <lib/types.h>
#include <kernel/screen.h>
#include <kernel/mboot.h>

void mboot_init(mboot_info_t *info)
{

    screen_puts("MULTIBOOT INFO\n");

    if (info->flags & MBOOT_FLAG_MEM)
    {

        screen_puts("Lower memory: ");
        screen_puts_dec(info->memoryLower);
        screen_puts("KB\n");

        screen_puts("Upper memory: ");
        screen_puts_dec(info->memoryUpper);
        screen_puts("KB\n");

    }

    if (info->flags & MBOOT_FLAG_DEVICE)
    {

        screen_puts("Device: 0x");
        screen_puts_hex(info->device);
        screen_puts("\n");

    }

    if (info->flags & MBOOT_FLAG_MMAP)
    {

        mboot_mmap_t *mmap = info->mmapAddress;

        screen_puts("Memorymap size: 0x");
        screen_puts_hex(mmap->size);
        screen_puts("\n");

        screen_puts("Memorymap base: 0x");
        screen_puts_hex(mmap->baseHigh);
        screen_puts_hex(mmap->baseLow);
        screen_puts("\n");

        screen_puts("Memorymap length: 0x");
        screen_puts_hex(mmap->lengthHigh);
        screen_puts_hex(mmap->lengthLow);
        screen_puts("\n");

        screen_puts("Memorymap type: 0x");
        screen_puts_hex(mmap->type);
        screen_puts("\n");

    }

    if (info->flags & MBOOT_FLAG_VBE)
    {


    }

}
