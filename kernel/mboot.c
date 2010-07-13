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

        screen_puts("Boot device: ");

        switch (info->device >> 24)
        {

            case 0xE0:
                screen_puts("CD-ROM\n");
                break;

            case 0x00:
                screen_puts("Floppy drive\n");
                break;

            case 0x80:
                screen_puts("Hard drive\n");
                break;

            default:
                screen_puts("Unknown (");
                screen_puts_hex(info->device >> 24);
                screen_puts(")\n");
                break;

        }

    }

    if (info->flags & MBOOT_FLAG_CMDLINE)
    {

        screen_puts("Command: ");
        screen_puts((char *)info->cmdline);
        screen_puts("\n");

    }

    if (info->flags & MBOOT_FLAG_MMAP)
    {

        mboot_mmap_t *mmap = (mboot_mmap_t *)info->mmapAddress;

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
