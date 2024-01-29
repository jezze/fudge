#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/arch.h>
#include "cpio.h"
#include "elf.h"
#include "mboot.h"

#define MBOOT_MAGIC                     0x2BADB002

void mboot_setup(struct mboot_header *header, unsigned int magic)
{

    arch_setup1();

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

    elf_setup();

    if (header->flags & MBOOT_FLAG_ELF)
    {


    }

    if (header->flags & MBOOT_FLAG_MODULES)
    {

        struct mboot_module *modules = (struct mboot_module *)header->modules.address;

        cpio_setup(modules[0].address, modules[0].limit);

    }

    if (header->flags & MBOOT_FLAG_MEMORY)
    {

    }

    if (header->flags & MBOOT_FLAG_MMAP)
    {

    }

    if (header->flags & MBOOT_FLAG_VBE)
    {

    }

    if (header->flags & MBOOT_FLAG_FRAMEBUFFER)
    {

    }

    arch_setup2();

}

