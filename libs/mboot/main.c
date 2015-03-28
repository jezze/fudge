#include <fudge.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <kernel/task.h>
#include <kernel/container.h>
#include <kernel/kernel.h>
#include <arch/x86/cpu.h>
#include <arch/x86/arch.h>
#include "mboot.h"

#define MBOOT_MAGIC                     0x2BADB002

static struct vfs_backend backend;
static unsigned long address;
static unsigned int limit;

static unsigned int read(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, (void *)address, limit, 1, offset);

}

static unsigned int write(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_write((void *)address, limit, buffer, count, 1, offset);

}

static unsigned long getphysical(struct vfs_backend *self)
{

    return address;

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

        struct mboot_module *modules = (struct mboot_module *)header->modules.address;

        vfs_initbackend(&backend, 1000, read, write, getphysical);

        address = modules[0].address;
        limit = modules[0].limit;

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

    arch_setup(&backend);

}

