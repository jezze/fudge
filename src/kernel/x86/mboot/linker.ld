ENTRY (mboot_init)

SECTIONS
{

    . = 0x00100000;

    .mboot : ALIGN(4096)
    {
        src/kernel/x86/mboot/init.o(.text.mboot)
    }

    .text : ALIGN(4096)
    {
        *(.text)
    }

    .data : ALIGN(4096)
    {
        *(.data)
        *(.rodata*)
        *(.bss)
    }

}
