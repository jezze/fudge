SECTIONS
{

    .text :
    {
        *(.text)
    }

    .data :
    {
        *(.data)
        *(.rodata)
        *(.bss)
    }

}
