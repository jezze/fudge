ENTRY (mboot_init)

SECTIONS
{

    . = 0x00100000;

    .mboot :
    {
        src/kernel/x86/mboot/init.o(.data)
    }

    .text :
    {
        *(.text)
    }

    .data :
    {
        *(.data)
        *(.rodata*)
        *(.bss)
    }

}
