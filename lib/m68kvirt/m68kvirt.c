#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <binary.h>
#include <disk.h>
#include <m68kvirt/cpio.h>
#include <m68kvirt/elf.h>

void m68kvirt_setup(void)
{

    struct cpio_header *header = (struct cpio_header *)ARCH_RAMDISK_BASE;

    arch_setup1();
    arch_kmap(ARCH_RAMDISK_BASE, ARCH_RAMDISK_BASE, ARCH_RAMDISK_SIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);
    elf_setup();
    arch_setup2();

    if (cpio_validate(header))
    {

        unsigned long init;

        cpio_setup(ARCH_RAMDISK_BASE);

        init = cpio_find("bin/init");

        if (init)
            arch_runinit(init);
        else
            debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "Init not found");

    }

    else
    {

        debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "Ramdisk not found");

    }

}

