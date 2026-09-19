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

unsigned long *UEFI_GOP = (unsigned long *)0x10000;
unsigned long *UEFI_ACPI = (unsigned long *)0x10008;

void mboot_setup(unsigned long address, unsigned long magic)
{

    struct mboot_tag_module *ramdisk = 0;
    struct mboot_tag_module *init = 0;
    struct mboot_tag *tag;

    *UEFI_GOP = 0;
    *UEFI_ACPI = 0;

    for (tag = (struct mboot_tag *)(address + 8); tag->type != MBOOT_TAG_END; tag = (struct mboot_tag *)((unsigned char *)tag + ((tag->size + 7) & ~7)))
    {

        if (tag->type == MBOOT_TAG_MODULE)
        {

            struct mboot_tag_module *module = (struct mboot_tag_module *)(tag + 1);
            struct elf_header *eheader = (struct elf_header *)module->start;
            struct cpio_header *cheader = (struct cpio_header *)module->start;

            if (elf_validate(eheader))
                init = module;

            else if (cpio_validate(cheader))
                ramdisk = module;

        }

        if (tag->type == MBOOT_TAG_FRAMEBUFFER)
        {

            struct mboot_tag_framebuffer *framebuffer = (struct mboot_tag_framebuffer *)(tag + 1);

            *UEFI_GOP = framebuffer->address[0];

        }

        if (tag->type == MBOOT_TAG_ACPI_OLD)
        {

            *UEFI_ACPI = *(unsigned long *)(tag + 1);

        }

        if (tag->type == MBOOT_TAG_ACPI_NEW)
        {

            *UEFI_ACPI = *(unsigned long *)(tag + 1);

        }

    }

    arch_setup1();
    elf_setup();

    if (ramdisk && init)
    {

        cpio_setup(ramdisk->start, ramdisk->end);
        arch_setup2(init->start);

    }

}

