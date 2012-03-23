SECTIONS
{

    .text :
    {
        *(.text)
    }

    .data ALIGN (0x1000) :
    {
        *(.data)
        *(.rodata)
        *(.bss)
    }

}
