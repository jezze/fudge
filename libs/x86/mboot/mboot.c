#include <fudge/memory.h>
#include <kernel/vfs.h>
#include <kernel/task.h>
#include <kernel/container.h>
#include <kernel/kernel.h>
#include <x86/kernel/arch.h>
#include "mboot.h"

static struct kernel_module modules[4];

static unsigned int read(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct kernel_module *module = (struct kernel_module *)self;

    return memory_read(buffer, count, module->address, module->limit, offset);

}

static unsigned int write(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct kernel_module *module = (struct kernel_module *)self;

    return memory_write(module->address, module->limit, buffer, count, offset);

}

void mboot_setup(struct mboot_header *header, unsigned int magic)
{

    if (header->flags & MBOOT_FLAG_LOADER)
    {

    }

    if (header->flags & MBOOT_FLAG_DEVICE)
    {

        unsigned int deviceNumber = header->device >> 24;

        switch (deviceNumber)
        {

            case 0xE0:

                break;

            case 0x00:

                break;

            case 0x80:

                break;

            default:

                break;

        }

    }

    if (header->flags & MBOOT_FLAG_CMDLINE)
    {

    }

    if (header->flags & MBOOT_FLAG_AOUT)
    {

    }

    if (header->flags & MBOOT_FLAG_ELF)
    {

    }

    if (header->flags & MBOOT_FLAG_MODULES)
    {

        struct mboot_module *mods = header->modules.address;
        unsigned int i;

        for (i = 0; i < header->modules.count; i++)
        {

            vfs_init_backend(&modules[i].base, read, write);

            modules[i].address = (void *)mods[i].address;
            modules[i].limit = mods[i].limit;

        }

    }

    if (header->flags & MBOOT_FLAG_MEMORY)
    {

    }

    if (header->flags & MBOOT_FLAG_MMAP)
    {

    }

    if (header->flags & MBOOT_FLAG_VBE)
    {

        if (header->vbe.cinfo)
        {

        }

    }

    arch_setup(header->modules.count, modules);

}

