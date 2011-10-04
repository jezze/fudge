#include <lib/memory.h>
#include <kernel/log.h>
#include <arch/x86/mboot.h>
#include <arch/x86/vbe.h>

void mboot_init(struct mboot_info *info)
{

    log_write("Multiboot information\n");
    log_write("=====================\n");

    if (info->flags & MBOOT_FLAG_LOADER)
    {

        log_write("Name: %s\n", (char *)info->name);

    }

    log_write("\n");

    if (info->flags & MBOOT_FLAG_MEMORY)
    {

        log_write("Memory\n");
        log_write("------\n");
        log_write("Lower memory: %dKiB\n", info->memoryLower);
        log_write("Upper memory: %dKiB\n", info->memoryUpper);
        log_write("\n");

    }

    if (info->flags & MBOOT_FLAG_DEVICE)
    {

        log_write("Boot device\n");
        log_write("-----------\n");

        unsigned int deviceNumber = info->device >> 24;

        log_write("Id: 0x%x\n", deviceNumber);

        switch (deviceNumber)
        {

            case 0xE0:

                log_write("Type: CD\n");

                break;

            case 0x00:

                log_write("Type: Floppy Disk\n");

                break;

            case 0x80:

                log_write("Type: Hard Drive\n");

                break;

            default:

                log_write("Type: Unknown\n");

                break;

        }

        log_write("\n");

    }

    log_write("Kernel\n");
    log_write("------\n");

    if (info->flags & MBOOT_FLAG_CMDLINE)
    {

        log_write("Arguments: %s\n", (char *)info->cmdline);

    }

    if (info->flags & MBOOT_FLAG_AOUT)
    {

        log_write("Format: AOUT\n");

    }

    if (info->flags & MBOOT_FLAG_ELF)
    {

        log_write("Format: ELF\n");

    }

    log_write("\n");

    if (info->flags & MBOOT_FLAG_MODULES)
    {

        log_write("Modules\n");
        log_write("-------\n");

        //struct mboot_module *module = (struct mboot_module *)info->modulesAddresses;

        //void *args[] = {(char *)module->name, &module->base, &module->length, &module->reserved};
        //log_message(LOG_TYPE_INFO, "Modules: %s Base: 0x%x Length: 0x%x Reserved:%d\n", args);

        log_write("\n");
    }

    if (info->flags & MBOOT_FLAG_MMAP)
    {

        log_write("Memory map\n");
        log_write("----------\n");

        struct mboot_mmap *mmap = (struct mboot_mmap *)info->mmapAddress;
/*
        log_message(LOG_TYPE_INFO, "Memory map\n", 0);

        while (mmap < info->mmapAddress + info->mmapLength)
        {

            void *args[] = {&mmap->baseHigh, &mmap->baseLow};

            if (mmap->type == 1)
                log_message(LOG_TYPE_INFO, "Available: BaseHigh: 0x%x BaseLow: 0x%x\n", args);
            else
                log_message(LOG_TYPE_INFO, "Reserved: BaseHigh: 0x%x BaseLow: 0x%x\n", args);

            mmap = (struct mboot_mmap *)((unsigned int)mmap + mmap->size + sizeof (unsigned int));

        }
*/

        log_write("\n");

    }

    if (info->flags & MBOOT_FLAG_VBE)
    {

        log_write("VESA BIOS Extension\n");
        log_write("-------------------\n");
        log_write("Controller info address: 0x%x\n", info->vbeControllerInfo);

        if (info->vbeControllerInfo)
        {

            struct vbe_controller_info *controller = (struct vbe_controller_info *)info->vbeControllerInfo;

            log_write("Signature: 0x%x\n", controller->version);

        }

        log_write("Mode info address: 0x%x\n", info->vbeModeInfo);
        log_write("Mode: 0x%x\n", info->vbeMode);
        log_write("Interface Segment: 0x%x\n", info->vbeInterfaceSegment);
        log_write("Interface Offset: 0x%x\n", info->vbeInterfaceOffset);
        log_write("Interface Length: 0x%x\n", info->vbeInterfaceLength);
        log_write("\n");

    }

}
