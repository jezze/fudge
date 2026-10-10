#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/arch.h>
#include <binary.h>
#include <disk.h>
#include "cpio.h"
#include "elf.h"
#include "mboot.h"

void mboot_setup(struct mboot_header *header, unsigned int magic)
{

    struct mboot_module ramdisk;
    struct mboot_module init;

    buffer_clear(&ramdisk, sizeof (struct mboot_module));
    buffer_clear(&init, sizeof (struct mboot_module));
    arch_setup1();

    if (header->flags & MBOOT_FLAG_MEMORY)
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

    if (header->flags & MBOOT_FLAG_MODULES)
    {

        struct mboot_module *modules = (struct mboot_module *)(unsigned long)header->modules.address;
        unsigned int i;

        for (i = 0; i < header->modules.count; i++)
        {

            struct elf_header *eheader = (struct elf_header *)(unsigned long)modules[i].start;
            struct cpio_header *cheader = (struct cpio_header *)(unsigned long)modules[i].start;

            arch_kmap(modules[i].start, modules[i].start, ((modules[i].end - modules[i].start) + 0xFFF) & ~0xFFF, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

            if (elf_validate(eheader))
                buffer_copy(&init, &modules[i], sizeof (struct mboot_module));

            else if (cpio_validate(cheader))
                buffer_copy(&ramdisk, &modules[i], sizeof (struct mboot_module));

        }

    }

    if (header->flags & MBOOT_FLAG_AOUT)
    {

    }

    if (header->flags & MBOOT_FLAG_ELF)
    {

    }

    if (header->flags & MBOOT_FLAG_MMAP)
    {

    }

    if (header->flags & MBOOT_FLAG_DRIVES)
    {

    }

    if (header->flags & MBOOT_FLAG_CONFIG)
    {

    }

    if (header->flags & MBOOT_FLAG_LOADER)
    {

    }

    if (header->flags & MBOOT_FLAG_APM)
    {

    }

    if (header->flags & MBOOT_FLAG_VBE)
    {

    }

    if (header->flags & MBOOT_FLAG_FRAMEBUFFER)
    {

    }

    elf_setup();
    arch_setup2();

    if (ramdisk.start && init.start)
    {

        cpio_setup(ramdisk.start);
        arch_runinit(init.start);

    }

}

