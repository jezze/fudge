ENTRY (init)

SECTIONS
{

    . = 0x00100000;

    .text :
    {
        *(.mboot)
        *(.text)
    }

    .data ALIGN (0x1000) :
    {
        *(.data)
        *(.rodata)
        *(.bss)
    }

}
