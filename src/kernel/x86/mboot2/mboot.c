#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/arch.h>
#include <binary.h>
#include <disk.h>
#include "cpio.h"
#include "elf.h"
#include "mboot.h"

void mboot_setup(unsigned long address, unsigned long magic)
{

    struct mboot_tag_module ramdisk;
    struct mboot_tag_module init;
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
        {

            /*
            struct mboot_tag_framebuffer *framebuffer = (struct mboot_tag_framebuffer *)(tag + 1);

            framebuffer->address[0];
            */

        }

        if (tag->type == MBOOT_TAG_ACPI_OLD)
        {


        }

        if (tag->type == MBOOT_TAG_ACPI_NEW)
        {

            /*
            buffer_copy(UEFI_ACPI, tag + 1, 36);
            */

        }

    }

    elf_setup();
    arch_setup2();

    if (ramdisk.start && init.start)
    {

        cpio_setup(ramdisk.start);
        arch_runinit(init.start);

    }

}

