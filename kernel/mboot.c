#include <kernel/mboot.h>

void mboot_init(struct mboot_info *info)
{

    arch_puts("System information\n");
    arch_puts("------------------\n");

    if (info->flags & MBOOT_FLAG_MEMORY)
    {

        arch_puts("Lower memory: ");
        arch_puts_dec(info->memoryLower);
        arch_puts("KB\n");

        arch_puts("Upper memory: ");
        arch_puts_dec(info->memoryUpper);
        arch_puts("KB\n");

    }

    if (info->flags & MBOOT_FLAG_DEVICE)
    {

        arch_puts("Boot device: ");

        switch (info->device >> 24)
        {

            case 0xE0:
                arch_puts("CD-ROM\n");
                break;

            case 0x00:
                arch_puts("Floppy drive\n");
                break;

            case 0x80:
                arch_puts("Hard drive\n");
                break;

            default:
                arch_puts("Unknown (");
                arch_puts_hex(info->device >> 24);
                arch_puts(")\n");
                break;

        }

    }

    if (info->flags & MBOOT_FLAG_CMDLINE)
    {

        arch_puts("Command: ");
        arch_puts((char *)info->cmdline);
        arch_puts("\n");

    }

    if (info->flags & MBOOT_FLAG_AOUT)
    {

        arch_puts("Kernel format: AOUT\n");

    }

    if (info->flags & MBOOT_FLAG_ELF)
    {

        arch_puts("Kernel format: ELF\n");

    }

    if (info->flags & MBOOT_FLAG_MODULES)
    {

        struct mboot_module *module = (struct mboot_module *)info->modulesAddresses;

        arch_puts("Modules:");

        arch_puts(" ");
        arch_puts((char *)module->name);
        arch_puts("\n");

    }

    if (info->flags & MBOOT_FLAG_MMAP)
    {

        struct mboot_mmap *mmap = (struct mboot_mmap *)info->mmapAddress;

        arch_puts("Memory map:\n");

        while (mmap < info->mmapAddress + info->mmapLength)
        {

            arch_puts("0x");

            if (mmap->baseHigh)
                arch_puts_hex(mmap->baseHigh);

            arch_puts_hex(mmap->baseLow);

            arch_puts("-0x");

            arch_puts_hex(mmap->baseLow + mmap->lengthLow);

            arch_puts(" (0x");

            if (mmap->lengthHigh)
                arch_puts_hex(mmap->lengthHigh);

            arch_puts_hex(mmap->lengthLow);

            arch_puts(" bytes) ");

            if (mmap->type == 1)
                arch_puts("Available");
            else
                arch_puts("Reserved");

            arch_puts("\n");

            mmap = (struct mboot_mmap *)((unsigned int)mmap + mmap->size + sizeof (unsigned int));

        }

    }

    if (info->flags & MBOOT_FLAG_LOADER)
    {

        arch_puts("Loader: ");
        arch_puts((char *)info->name);
        arch_puts("\n");

    }

    if (info->flags & MBOOT_FLAG_VBE)
    {


    }

    arch_puts("\n");

}
