SECTIONS
{

    . = 0x00000000;

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
