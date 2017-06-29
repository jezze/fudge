ENTRY (mboot_init)

SECTIONS
{

    . = 0x00100000;

    .mboot :
    {
        lib/mboot/init.o(.data)
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
