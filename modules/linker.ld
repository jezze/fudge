SECTIONS
{

    .text ALIGN (0x1000) :
    {
        *(.text)
    }

    .data ALIGN (0x1000) :
    {
        *(.rodata*)
        *(.data)
    }

    .bss :
    {
        *(.bss)
    }

}
