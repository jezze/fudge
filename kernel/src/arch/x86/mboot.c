#include <kernel.h>
#include <runtime.h>
#include <arch/x86/arch.h>
#include <arch/x86/cpu.h>
#include <arch/x86/isr.h>
#include <arch/x86/mboot.h>

void mboot_setup(struct mboot_header *header, unsigned int magic)
{

    struct runtime_task *task;

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

    arch_setup();

    task = kernel_setup(header->modules.count, header->modules.address);

    isr_set_task(task);

    cpu_enter_usermode(task->registers.ip, task->registers.sp);

}

