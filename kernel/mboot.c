#include <lib/write.h>
#include <kernel/mboot.h>

void mboot_init(struct mboot_info *info)
{

    write_string("System information\n");
    write_string("------------------\n");

    if (info->flags & MBOOT_FLAG_MEMORY)
    {

        write_string("Lower memory: ");
        write_string_dec(info->memoryLower);
        write_string("KB\n");

        write_string("Upper memory: ");
        write_string_dec(info->memoryUpper);
        write_string("KB\n");

    }

    if (info->flags & MBOOT_FLAG_DEVICE)
    {

        write_string("Boot device: ");

        switch (info->device >> 24)
        {

            case 0xE0:
                write_string("CD-ROM\n");
                break;

            case 0x00:
                write_string("Floppy drive\n");
                break;

            case 0x80:
                write_string("Hard drive\n");
                break;

            default:
                write_string("Unknown (");
                write_string_hex(info->device >> 24);
                write_string(")\n");
                break;

        }

    }

    if (info->flags & MBOOT_FLAG_CMDLINE)
    {

        write_string("Command: ");
        write_string((char *)info->cmdline);
        write_string("\n");

    }

    if (info->flags & MBOOT_FLAG_AOUT)
    {

        write_string("Kernel format: AOUT\n");

    }

    if (info->flags & MBOOT_FLAG_ELF)
    {

        write_string("Kernel format: ELF\n");

    }

    if (info->flags & MBOOT_FLAG_MODULES)
    {

        struct mboot_module *module = (struct mboot_module *)info->modulesAddresses;

        write_string("Modules: ");
        write_string((char *)module->name);
        write_string(" Base: 0x");
        write_string_hex(module->base);
        write_string(" Length: 0x");
        write_string_hex(module->length);
        write_string(" Reserved: ");
        write_string_dec(module->reserved);
        write_string("\n");

    }

    if (info->flags & MBOOT_FLAG_MMAP)
    {

        struct mboot_mmap *mmap = (struct mboot_mmap *)info->mmapAddress;

        write_string("Memory map:\n");

        while (mmap < info->mmapAddress + info->mmapLength)
        {

            write_string("0x");

            if (mmap->baseHigh)
                write_string_hex(mmap->baseHigh);

            write_string_hex(mmap->baseLow);

            write_string("-0x");

            write_string_hex(mmap->baseLow + mmap->lengthLow);

            write_string(" (0x");

            if (mmap->lengthHigh)
                write_string_hex(mmap->lengthHigh);

            write_string_hex(mmap->lengthLow);

            write_string(" bytes) ");

            if (mmap->type == 1)
                write_string("Available");
            else
                write_string("Reserved");

            write_string("\n");

            mmap = (struct mboot_mmap *)((unsigned int)mmap + mmap->size + sizeof (unsigned int));

        }

    }

    if (info->flags & MBOOT_FLAG_LOADER)
    {

        write_string("Loader: ");
        write_string((char *)info->name);
        write_string("\n");

    }

    if (info->flags & MBOOT_FLAG_VBE)
    {


    }

    write_string("\n");

}
