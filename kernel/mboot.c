#include <lib/file.h>
#include <lib/session.h>
#include <kernel/mboot.h>

void mboot_init(struct mboot_info *info)
{

    file_write(session_get_out(), "System information\n");
    file_write(session_get_out(), "------------------\n");

    if (info->flags & MBOOT_FLAG_MEMORY)
    {

        file_write(session_get_out(), "Lower memory: ");
        file_write_dec(session_get_out(), info->memoryLower);
        file_write(session_get_out(), "KB\n");

        file_write(session_get_out(), "Upper memory: ");
        file_write_dec(session_get_out(), info->memoryUpper);
        file_write(session_get_out(), "KB\n");

    }

    if (info->flags & MBOOT_FLAG_DEVICE)
    {

        file_write(session_get_out(), "Boot device: ");

        switch (info->device >> 24)
        {

            case 0xE0:
                file_write(session_get_out(), "CD-ROM\n");
                break;

            case 0x00:
                file_write(session_get_out(), "Floppy drive\n");
                break;

            case 0x80:
                file_write(session_get_out(), "Hard drive\n");
                break;

            default:
                file_write(session_get_out(), "Unknown (");
                file_write_hex(session_get_out(), info->device >> 24);
                file_write(session_get_out(), ")\n");
                break;

        }

    }

    if (info->flags & MBOOT_FLAG_CMDLINE)
    {

        file_write(session_get_out(), "Command: ");
        file_write(session_get_out(), (char *)info->cmdline);
        file_write(session_get_out(), "\n");

    }

    if (info->flags & MBOOT_FLAG_AOUT)
    {

        file_write(session_get_out(), "Kernel format: AOUT\n");

    }

    if (info->flags & MBOOT_FLAG_ELF)
    {

        file_write(session_get_out(), "Kernel format: ELF\n");

    }

    if (info->flags & MBOOT_FLAG_MODULES)
    {

        struct mboot_module *module = (struct mboot_module *)info->modulesAddresses;

        file_write(session_get_out(), "Modules: ");
        file_write(session_get_out(), (char *)module->name);
        file_write(session_get_out(), " Base: 0x");
        file_write_hex(session_get_out(), module->base);
        file_write(session_get_out(), " Length: 0x");
        file_write_hex(session_get_out(), module->length);
        file_write(session_get_out(), " Reserved: ");
        file_write_dec(session_get_out(), module->reserved);
        file_write(session_get_out(), "\n");

    }

    if (info->flags & MBOOT_FLAG_MMAP)
    {

        struct mboot_mmap *mmap = (struct mboot_mmap *)info->mmapAddress;

        file_write(session_get_out(), "Memory map:\n");

        while (mmap < info->mmapAddress + info->mmapLength)
        {

            file_write(session_get_out(), "0x");

            if (mmap->baseHigh)
                file_write_hex(session_get_out(), mmap->baseHigh);

            file_write_hex(session_get_out(), mmap->baseLow);

            file_write(session_get_out(), "-0x");

            file_write_hex(session_get_out(), mmap->baseLow + mmap->lengthLow);

            file_write(session_get_out(), " (0x");

            if (mmap->lengthHigh)
                file_write_hex(session_get_out(), mmap->lengthHigh);

            file_write_hex(session_get_out(), mmap->lengthLow);

            file_write(session_get_out(), " bytes) ");

            if (mmap->type == 1)
                file_write(session_get_out(), "Available");
            else
                file_write(session_get_out(), "Reserved");

            file_write(session_get_out(), "\n");

            mmap = (struct mboot_mmap *)((unsigned int)mmap + mmap->size + sizeof (unsigned int));

        }

    }

    if (info->flags & MBOOT_FLAG_LOADER)
    {

        file_write(session_get_out(), "Loader: ");
        file_write(session_get_out(), (char *)info->name);
        file_write(session_get_out(), "\n");

    }

    if (info->flags & MBOOT_FLAG_VBE)
    {


    }

    file_write(session_get_out(), "\n");

}
