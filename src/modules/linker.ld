SECTIONS
{

    . = 0x00000000;

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
