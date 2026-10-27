#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <udebug.h>
#include <binary.h>
#include <disk.h>
#include <amiga/cpio.h>
#include <amiga/elf.h>

/* fudgeboot leaves the ramdisk right after the kernel, below ARCH_RAMDISK_BASE */
void amiga_setup(unsigned long ramdisk, unsigned int size)
{

    struct cpio_header *header = (struct cpio_header *)ARCH_RAMDISK_BASE;

    if (size <= ARCH_RAMDISK_SIZE)
        buffer_copy(header, (void *)ramdisk, size);

    arch_setup1();

    if (size > ARCH_RAMDISK_SIZE)
        debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "Ramdisk too big");

    arch_kmap(ARCH_RAMDISK_BASE, ARCH_RAMDISK_BASE, ARCH_RAMDISK_SIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);
    elf_setup();
    arch_setup2();
    udebug_setbackground(0x0060);

    if (cpio_validate(header))
    {

        unsigned long init;

        cpio_setup(ARCH_RAMDISK_BASE);

        init = cpio_find("bin/init");

        if (init)
        {

            udebug_setbackground(0x0606);
            arch_runinit(init);

        }

        else
            debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "Init not found");

    }

    else
    {

        debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "Ramdisk not found");

    }

}

