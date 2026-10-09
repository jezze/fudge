#include <fudge.h>
#include <kernel.h>
#include <kernel/x86_64/cpu.h>
#include <kernel/x86_64/arch.h>
#include <binary.h>
#include <disk.h>
#include "cpio.h"
#include "elf.h"
#include "mboot.h"

void mboot_setup(unsigned long address, unsigned long magic)
{

    struct mboot_tag_module ramdisk;
    struct mboot_tag_module init;
    struct mboot_tag_framebuffer *framebuffer = 0;
    unsigned int efi = 0;
    struct mboot_tag *tag;

    buffer_clear(&ramdisk, sizeof (struct mboot_tag_module));
    buffer_clear(&init, sizeof (struct mboot_tag_module));
    arch_setup1();

    for (tag = (struct mboot_tag *)(address + 8); tag->type != MBOOT_TAG_END; tag = (struct mboot_tag *)((unsigned char *)tag + ((tag->size + 7) & ~7)))
    {

        if (tag->type == MBOOT_TAG_MODULE)
        {

            struct mboot_tag_module *module = (struct mboot_tag_module *)(tag + 1);
            struct elf_header *eheader = (struct elf_header *)(unsigned long)module->start;
            struct cpio_header *cheader = (struct cpio_header *)(unsigned long)module->start;

            arch_kmap(module->start, module->start, ((module->end - module->start) + 0xFFF) & ~0xFFF, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

            if (elf_validate(eheader))
                buffer_copy(&init, module, sizeof (struct mboot_tag_module));

            else if (cpio_validate(cheader))
                buffer_copy(&ramdisk, module, sizeof (struct mboot_tag_module));

        }

        if (tag->type == MBOOT_TAG_FRAMEBUFFER)
            framebuffer = (struct mboot_tag_framebuffer *)(tag + 1);

        if (tag->type == MBOOT_TAG_EFI32 || tag->type == MBOOT_TAG_EFI64)
            efi = 1;

        if (tag->type == MBOOT_TAG_ACPI_OLD)
        {

            if (!*(unsigned char *)ARCH_FIRMWARE_ACPI)
                buffer_copy((void *)ARCH_FIRMWARE_ACPI, tag + 1, tag->size - sizeof (struct mboot_tag));

        }

        if (tag->type == MBOOT_TAG_ACPI_NEW)
        {

            buffer_copy((void *)ARCH_FIRMWARE_ACPI, tag + 1, tag->size - sizeof (struct mboot_tag));

        }

    }

    if (efi && framebuffer && framebuffer->type == MBOOT_FRAMEBUFFER_TYPE_RGB && !framebuffer->address[1])
    {

        struct mboot_tag_framebuffer_rgb *rgb = (struct mboot_tag_framebuffer_rgb *)(framebuffer + 1);

        if (rgb->redposition == 16 && rgb->greenposition == 8 && rgb->blueposition == 0)
        {

            struct arch_framebuffer *firmware = (struct arch_framebuffer *)ARCH_FIRMWARE_FRAMEBUFFER;

            firmware->address = framebuffer->address[0];
            firmware->width = framebuffer->width;
            firmware->height = framebuffer->height;
            firmware->pitch = framebuffer->pitch;
            firmware->bpp = framebuffer->bpp;

        }

    }

    elf_setup();
    arch_setup2();

    if (ramdisk.start && init.start)
    {

        cpio_setup(ramdisk.start);
        arch_runinit(init.start);

    }

    else
    {

        debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "Ramdisk or init not found");

    }

}

