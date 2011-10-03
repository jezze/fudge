#include <kernel/log.h>
#include <arch/x86/mboot.h>

void mboot_init(struct mboot_info *info)
{

    log_message(LOG_TYPE_INFO, "System information\n", 0);

    if (info->flags & MBOOT_FLAG_MEMORY)
    {

        void *args1[] = {&info->memoryLower};
        log_message(LOG_TYPE_INFO, "Lower memory: %dKB\n", args1);

        void *args2[] = {&info->memoryUpper};
        log_message(LOG_TYPE_INFO, "Upper memory: %dKB\n", args2);

    }

    if (info->flags & MBOOT_FLAG_DEVICE)
    {

        unsigned int deviceNumber = info->device >> 24;

        log_message(LOG_TYPE_INFO, "Boot devices\n", 0);

        switch (deviceNumber)
        {

            case 0xE0:

                log_message(LOG_TYPE_INFO, "CD-ROM\n", 0);

                break;

            case 0x00:

                log_message(LOG_TYPE_INFO, "Floppy drive\n", 0);

                break;

            case 0x80:

                log_message(LOG_TYPE_INFO, "Hard drive\n", 0);

                break;

            default:

                //void *args[] = {&deviceNumber};
                //log_message(LOG_TYPE_INFO, "Unknown (%d)", args);

                break;

        }

    }

    if (info->flags & MBOOT_FLAG_CMDLINE)
    {

        void *args[] = {(char *)info->cmdline};
        log_message(LOG_TYPE_INFO, "Command: %s\n", args);

    }

    if (info->flags & MBOOT_FLAG_AOUT)
    {

        log_message(LOG_TYPE_INFO, "Kernel format: AOUT\n", 0);

    }

    if (info->flags & MBOOT_FLAG_ELF)
    {

        log_message(LOG_TYPE_INFO, "Kernel format: ELF\n", 0);

    }

    if (info->flags & MBOOT_FLAG_MODULES)
    {

        struct mboot_module *module = (struct mboot_module *)info->modulesAddresses;

        void *args[] = {(char *)module->name, &module->base, &module->length, &module->reserved};
        log_message(LOG_TYPE_INFO, "Modules: %s Base: 0x%x Length: 0x%x Reserved:%d\n", args);

    }

    if (info->flags & MBOOT_FLAG_MMAP)
    {

        struct mboot_mmap *mmap = (struct mboot_mmap *)info->mmapAddress;

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

    }

    if (info->flags & MBOOT_FLAG_LOADER)
    {

        void *args[] = {(char *)info->name};
        log_message(LOG_TYPE_INFO, "Loader: %s\n", args);

    }

    if (info->flags & MBOOT_FLAG_VBE)
    {


    }

}
