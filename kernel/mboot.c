#include <kernel/screen.h>
#include <kernel/mboot.h>

void mboot_init(struct mboot_info *info)
{

    screen_puts("System information\n");
    screen_puts("------------------\n");

    if (info->flags & MBOOT_FLAG_MEMORY)
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

    if (info->flags & MBOOT_FLAG_AOUT)
    {

        screen_puts("Kernel format: AOUT\n");

    }

    if (info->flags & MBOOT_FLAG_ELF)
    {

        screen_puts("Kernel format: ELF\n");

    }

    if (info->flags & MBOOT_FLAG_MODULES)
    {

        struct mboot_module *module = (struct mboot_module *)info->modulesAddresses;

        screen_puts("Modules:");

        screen_puts(" ");
        screen_puts((char *)module->name);
        screen_puts("\n");

    }

    if (info->flags & MBOOT_FLAG_MMAP)
    {

        struct mboot_mmap *mmap = (struct mboot_mmap *)info->mmapAddress;

        screen_puts("Memory map:\n");

        while (mmap < info->mmapAddress + info->mmapLength)
        {

            screen_puts("0x");

            if (mmap->baseHigh)
                screen_puts_hex(mmap->baseHigh);

            screen_puts_hex(mmap->baseLow);

            screen_puts("-0x");

            screen_puts_hex(mmap->baseLow + mmap->lengthLow);

            screen_puts(" (0x");

            if (mmap->lengthHigh)
                screen_puts_hex(mmap->lengthHigh);

            screen_puts_hex(mmap->lengthLow);

            screen_puts(" bytes) ");

            if (mmap->type == 1)
                screen_puts("Available");
            else
                screen_puts("Reserved");

            screen_puts("\n");

            mmap = (struct mboot_mmap *)((unsigned int)mmap + mmap->size + sizeof (unsigned int));

        }

    }

    if (info->flags & MBOOT_FLAG_LOADER)
    {

        screen_puts("Loader: ");
        screen_puts((char *)info->name);
        screen_puts("\n");

    }

    if (info->flags & MBOOT_FLAG_VBE)
    {


    }

    screen_puts("\n");

}
