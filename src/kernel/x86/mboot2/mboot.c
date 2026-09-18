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

void mboot_setup(unsigned long address, unsigned long magic)
{

    struct mboot_tag_module *ramdisk = 0;
    struct mboot_tag_module *init = 0;
    unsigned int nmodules = 0;
    struct mboot_tag *tag;

    arch_setup1();
    elf_setup();

    for (tag = (struct mboot_tag *)(address + 8); tag->type != MBOOT_TAG_END; tag = (struct mboot_tag *)((unsigned char *)tag + ((tag->size + 7) & ~7)))
    {

        if (tag->type == MBOOT_TAG_MODULE)
        {

            struct mboot_tag_module *module = (struct mboot_tag_module *)(tag + 1);

            switch (nmodules)
            {

            case 0:
                ramdisk = module;

                break;

            case 1:
                init = module;

                break;

            }

            nmodules++;

        }

        if (tag->type == MBOOT_TAG_FRAMEBUFFER)
        {

        }

    }

    if (ramdisk)
        cpio_setup(ramdisk->start, ramdisk->end);

    if (init)
        arch_setup2(init->start);

    for (;;);

}

