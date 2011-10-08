#include <lib/memory.h>
#include <kernel/log.h>
#include <arch/x86/mboot.h>
#include <arch/x86/vbe.h>

void mboot_init(struct mboot_info *info)
{

    if (info->flags & MBOOT_FLAG_LOADER)
    {

        log_write("[mboot] Bootloader: %s\n", (char *)info->name);

    }

    if (info->flags & MBOOT_FLAG_MEMORY)
    {

        log_write("[mboot] Lower memory: %dKiB\n", info->memoryLower);
        log_write("[mboot] Upper memory: %dKiB\n", info->memoryUpper);

    }

    if (info->flags & MBOOT_FLAG_DEVICE)
    {

        unsigned int deviceNumber = info->device >> 24;

        log_write("[mboot] ");

        switch (deviceNumber)
        {

            case 0xE0:

                log_write("Device: CD\n");

                break;

            case 0x00:

                log_write("Device: Floppy Disk\n");

                break;

            case 0x80:

                log_write("Device: Hard Drive\n");

                break;

            default:

                log_write("Device: Unknown\n");

                break;

        }

    }

    if (info->flags & MBOOT_FLAG_CMDLINE)
    {

        log_write("[mboot] Kernel arguments: %s\n", (char *)info->cmdline);

    }

    if (info->flags & MBOOT_FLAG_AOUT)
    {

        log_write("[mboot] Kernel format: AOUT\n");

    }

    if (info->flags & MBOOT_FLAG_ELF)
    {

        log_write("[mboot] Kernel format: ELF\n");

    }

    if (info->flags & MBOOT_FLAG_MODULES)
    {

        //struct mboot_module *module = (struct mboot_module *)info->modulesAddresses;

        //void *args[] = {(char *)module->name, &module->base, &module->length, &module->reserved};
        //log_message(LOG_TYPE_INFO, "Modules: %s Base: 0x%x Length: 0x%x Reserved:%d\n", args);

    }

    if (info->flags & MBOOT_FLAG_MMAP)
    {

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

    }

    if (info->flags & MBOOT_FLAG_VBE)
    {

        log_write("[mboot] VBE controller address: 0x%x\n", info->vbeControllerInfo);
        log_write("[mboot] VBE mode address: 0x%x\n", info->vbeModeInfo);

        if (info->vbeControllerInfo)
        {

            struct vbe_controller_info *controller = (struct vbe_controller_info *)info->vbeControllerInfo;

            log_write("[mboot] VBE controller signature: 0x%x\n", controller->version);

        }

        log_write("[mboot] Mode: 0x%x\n", info->vbeMode);
        log_write("[mboot] Interface Segment: 0x%x\n", info->vbeInterfaceSegment);
        log_write("[mboot] Interface Offset: 0x%x\n", info->vbeInterfaceOffset);
        log_write("[mboot] Interface Length: 0x%x\n", info->vbeInterfaceLength);

    }

}

