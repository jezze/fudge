#include <lib/stdout.h>
#include <kernel/mboot.h>

void mboot_init(struct mboot_info *info)
{

    stdout_write("System information\n");
    stdout_write("------------------\n");

    if (info->flags & MBOOT_FLAG_MEMORY)
    {

        stdout_write("Lower memory: ");
        stdout_write_dec(info->memoryLower);
        stdout_write("KB\n");

        stdout_write("Upper memory: ");
        stdout_write_dec(info->memoryUpper);
        stdout_write("KB\n");

    }

    if (info->flags & MBOOT_FLAG_DEVICE)
    {

        stdout_write("Boot device: ");

        switch (info->device >> 24)
        {

            case 0xE0:
                stdout_write("CD-ROM\n");
                break;

            case 0x00:
                stdout_write("Floppy drive\n");
                break;

            case 0x80:
                stdout_write("Hard drive\n");
                break;

            default:
                stdout_write("Unknown (");
                stdout_write_hex(info->device >> 24);
                stdout_write(")\n");
                break;

        }

    }

    if (info->flags & MBOOT_FLAG_CMDLINE)
    {

        stdout_write("Command: ");
        stdout_write((char *)info->cmdline);
        stdout_write("\n");

    }

    if (info->flags & MBOOT_FLAG_AOUT)
    {

        stdout_write("Kernel format: AOUT\n");

    }

    if (info->flags & MBOOT_FLAG_ELF)
    {

        stdout_write("Kernel format: ELF\n");

    }

    if (info->flags & MBOOT_FLAG_MODULES)
    {

        struct mboot_module *module = (struct mboot_module *)info->modulesAddresses;

        stdout_write("Modules: ");
        stdout_write((char *)module->name);
        stdout_write(" Base: 0x");
        stdout_write_hex(module->base);
        stdout_write(" Length: 0x");
        stdout_write_hex(module->length);
        stdout_write(" Reserved: ");
        stdout_write_dec(module->reserved);
        stdout_write("\n");

    }

    if (info->flags & MBOOT_FLAG_MMAP)
    {

        struct mboot_mmap *mmap = (struct mboot_mmap *)info->mmapAddress;

        stdout_write("Memory map:\n");

        while (mmap < info->mmapAddress + info->mmapLength)
        {

            stdout_write("0x");

            if (mmap->baseHigh)
                stdout_write_hex(mmap->baseHigh);

            stdout_write_hex(mmap->baseLow);

            stdout_write("-0x");

            stdout_write_hex(mmap->baseLow + mmap->lengthLow);

            stdout_write(" (0x");

            if (mmap->lengthHigh)
                stdout_write_hex(mmap->lengthHigh);

            stdout_write_hex(mmap->lengthLow);

            stdout_write(" bytes) ");

            if (mmap->type == 1)
                stdout_write("Available");
            else
                stdout_write("Reserved");

            stdout_write("\n");

            mmap = (struct mboot_mmap *)((unsigned int)mmap + mmap->size + sizeof (unsigned int));

        }

    }

    if (info->flags & MBOOT_FLAG_LOADER)
    {

        stdout_write("Loader: ");
        stdout_write((char *)info->name);
        stdout_write("\n");

    }

    if (info->flags & MBOOT_FLAG_VBE)
    {


    }

    stdout_write("\n");

}
