#include <lib/memory.h>
#include <kernel/log.h>
#include <kernel/arch/x86/mboot.h>
#include <kernel/arch/x86/vbe.h>

void mboot_init(struct mboot_info *info)
{

    if (info->flags & MBOOT_FLAG_LOADER)
    {

        log_write("[mboot] Boot loader: %s\n", (char *)info->name);

    }

    if (info->flags & MBOOT_FLAG_DEVICE)
    {

        unsigned int deviceNumber = info->device >> 24;

        log_write("[mboot] Boot device: ");

        switch (deviceNumber)
        {

            case 0xE0:

                log_write("CD\n");

                break;

            case 0x00:

                log_write("Floppy Disk\n");

                break;

            case 0x80:

                log_write("Hard Drive\n");

                break;

            default:

                log_write("Device: Unknown\n");

                break;

        }

    }

    if (info->flags & MBOOT_FLAG_CMDLINE)
    {

        log_write("[mboot] Kernel: %s\n", (char *)info->cmdline);

    }

    if (info->flags & MBOOT_FLAG_AOUT)
    {

        log_write("[mboot] Kernel type: AOUT\n");

    }

    if (info->flags & MBOOT_FLAG_ELF)
    {

        log_write("[mboot] Kernel type: ELF\n");

    }

    if (info->flags & MBOOT_FLAG_MODULES)
    {

        struct mboot_module *module = (struct mboot_module *)info->modules.address;

        log_write("[mboot] Module: %s\n", (char *)module->name);
        log_write("[mboot] Module address: 0x%x\n", module->base);

    }

    if (info->flags & MBOOT_FLAG_MEMORY)
    {

        log_write("[mboot] Lower memory: %dKiB\n", info->mlower);
        log_write("[mboot] Upper memory: %dKiB\n", info->mupper);

    }

    if (info->flags & MBOOT_FLAG_MMAP)
    {

        void *address = info->mmap.address;
        unsigned int total = (unsigned int)address + info->mmap.count;

        while (address < total)
        {

            struct mboot_mmap *mmap = (struct mboot_mmap *)address;

            if (mmap->type == 1)
                log_write("[mboot] Memory available: 0x%x:0x%x\n", mmap->blow, mmap->llow);
            else
                log_write("[mboot] Memory reserved: 0x%x:0x%x\n", mmap->blow, mmap->llow);

            address += mmap->size + sizeof (unsigned int);

        }

    }

    if (info->flags & MBOOT_FLAG_VBE)
    {

        log_write("[mboot] VBE ctrl address: 0x%x\n", info->vbe.cinfo);

        if (info->vbe.cinfo)
        {

            struct vbe_controller_info *controller = (struct vbe_controller_info *)info->vbe.cinfo;

            log_write("[mboot] VBE ctrl signature: 0x%x\n", controller->version);

        }

        log_write("[mboot] VBE mode address: 0x%x\n", info->vbe.minfo);
        log_write("[mboot] VBE interface segment: 0x%x\n", info->vbe.isegment);
        log_write("[mboot] VBE interface offset: 0x%x\n", info->vbe.ioffset);
        log_write("[mboot] VBE interface length: 0x%x\n", info->vbe.ilength);

    }

}

