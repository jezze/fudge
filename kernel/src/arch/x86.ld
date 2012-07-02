ENTRY (init)

SECTIONS
{

    . = 0x00100000;

    .text ALIGN (0x1000) :
    {
        *(.mboot)
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
