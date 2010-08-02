#include <lib/call.h>
#include <kernel/mboot.h>

void mboot_init(struct mboot_info *info)
{

    call_puts("System information\n");
    call_puts("------------------\n");

    if (info->flags & MBOOT_FLAG_MEMORY)
    {

        call_puts("Lower memory: ");
        call_puts_dec(info->memoryLower);
        call_puts("KB\n");

        call_puts("Upper memory: ");
        call_puts_dec(info->memoryUpper);
        call_puts("KB\n");

    }

    if (info->flags & MBOOT_FLAG_DEVICE)
    {

        call_puts("Boot device: ");

        switch (info->device >> 24)
        {

            case 0xE0:
                call_puts("CD-ROM\n");
                break;

            case 0x00:
                call_puts("Floppy drive\n");
                break;

            case 0x80:
                call_puts("Hard drive\n");
                break;

            default:
                call_puts("Unknown (");
                call_puts_hex(info->device >> 24);
                call_puts(")\n");
                break;

        }

    }

    if (info->flags & MBOOT_FLAG_CMDLINE)
    {

        call_puts("Command: ");
        call_puts((char *)info->cmdline);
        call_puts("\n");

    }

    if (info->flags & MBOOT_FLAG_AOUT)
    {

        call_puts("Kernel format: AOUT\n");

    }

    if (info->flags & MBOOT_FLAG_ELF)
    {

        call_puts("Kernel format: ELF\n");

    }

    if (info->flags & MBOOT_FLAG_MODULES)
    {

        struct mboot_module *module = (struct mboot_module *)info->modulesAddresses;

        call_puts("Modules:");

        call_puts(" ");
        call_puts((char *)module->name);
        call_puts("\n");

    }

    if (info->flags & MBOOT_FLAG_MMAP)
    {

        struct mboot_mmap *mmap = (struct mboot_mmap *)info->mmapAddress;

        call_puts("Memory map:\n");

        while (mmap < info->mmapAddress + info->mmapLength)
        {

            call_puts("0x");

            if (mmap->baseHigh)
                call_puts_hex(mmap->baseHigh);

            call_puts_hex(mmap->baseLow);

            call_puts("-0x");

            call_puts_hex(mmap->baseLow + mmap->lengthLow);

            call_puts(" (0x");

            if (mmap->lengthHigh)
                call_puts_hex(mmap->lengthHigh);

            call_puts_hex(mmap->lengthLow);

            call_puts(" bytes) ");

            if (mmap->type == 1)
                call_puts("Available");
            else
                call_puts("Reserved");

            call_puts("\n");

            mmap = (struct mboot_mmap *)((unsigned int)mmap + mmap->size + sizeof (unsigned int));

        }

    }

    if (info->flags & MBOOT_FLAG_LOADER)
    {

        call_puts("Loader: ");
        call_puts((char *)info->name);
        call_puts("\n");

    }

    if (info->flags & MBOOT_FLAG_VBE)
    {


    }

    call_puts("\n");

}
