ENTRY (init)

SECTIONS
{

    . = 0x00100000;

    .text :
    {
        *(.text)
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss)
    }

}
