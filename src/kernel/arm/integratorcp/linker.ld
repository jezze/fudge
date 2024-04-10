ENTRY (init)

SECTIONS
{

    . = 0x00010000;

    .text :
    {
        *(.text.boot)
        *(.text)
    }

    .data :
    {
        *(.data)
        *(.rodata*)
        *(.bss)
    }

}
