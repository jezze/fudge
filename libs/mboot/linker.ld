ENTRY (init)

SECTIONS
{

    . = 0x00100000;

    .text :
    {
        *(.mboot)
        *(.text)
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
