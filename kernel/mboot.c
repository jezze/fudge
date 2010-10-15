#include <lib/file.h>
#include <kernel/mboot.h>

void mboot_init(struct mboot_info *info)
{

    file_write("System information\n");
    file_write("------------------\n");

    if (info->flags & MBOOT_FLAG_MEMORY)
    {

        file_write("Lower memory: ");
        file_write_dec(info->memoryLower);
        file_write("KB\n");

        file_write("Upper memory: ");
        file_write_dec(info->memoryUpper);
        file_write("KB\n");

    }

    if (info->flags & MBOOT_FLAG_DEVICE)
    {

        file_write("Boot device: ");

        switch (info->device >> 24)
        {

            case 0xE0:
                file_write("CD-ROM\n");
                break;

            case 0x00:
                file_write("Floppy drive\n");
                break;

            case 0x80:
                file_write("Hard drive\n");
                break;

            default:
                file_write("Unknown (");
                file_write_hex(info->device >> 24);
                file_write(")\n");
                break;

        }

    }

    if (info->flags & MBOOT_FLAG_CMDLINE)
    {

        file_write("Command: ");
        file_write((char *)info->cmdline);
        file_write("\n");

    }

    if (info->flags & MBOOT_FLAG_AOUT)
    {

        file_write("Kernel format: AOUT\n");

    }

    if (info->flags & MBOOT_FLAG_ELF)
    {

        file_write("Kernel format: ELF\n");

    }

    if (info->flags & MBOOT_FLAG_MODULES)
    {

        struct mboot_module *module = (struct mboot_module *)info->modulesAddresses;

        file_write("Modules: ");
        file_write((char *)module->name);
        file_write(" Base: 0x");
        file_write_hex(module->base);
        file_write(" Length: 0x");
        file_write_hex(module->length);
        file_write(" Reserved: ");
        file_write_dec(module->reserved);
        file_write("\n");

    }

    if (info->flags & MBOOT_FLAG_MMAP)
    {

        struct mboot_mmap *mmap = (struct mboot_mmap *)info->mmapAddress;

        file_write("Memory map:\n");

        while (mmap < info->mmapAddress + info->mmapLength)
        {

            file_write("0x");

            if (mmap->baseHigh)
                file_write_hex(mmap->baseHigh);

            file_write_hex(mmap->baseLow);

            file_write("-0x");

            file_write_hex(mmap->baseLow + mmap->lengthLow);

            file_write(" (0x");

            if (mmap->lengthHigh)
                file_write_hex(mmap->lengthHigh);

            file_write_hex(mmap->lengthLow);

            file_write(" bytes) ");

            if (mmap->type == 1)
                file_write("Available");
            else
                file_write("Reserved");

            file_write("\n");

            mmap = (struct mboot_mmap *)((unsigned int)mmap + mmap->size + sizeof (unsigned int));

        }

    }

    if (info->flags & MBOOT_FLAG_LOADER)
    {

        file_write("Loader: ");
        file_write((char *)info->name);
        file_write("\n");

    }

    if (info->flags & MBOOT_FLAG_VBE)
    {


    }

    file_write("\n");

}
