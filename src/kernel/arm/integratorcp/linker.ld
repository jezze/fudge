ENTRY (init)

SECTIONS
{

    . = 0x00010000;

    .text : ALIGN(4096)
    {
        *(.text.boot)
        *(.text)
    }

    .data : ALIGN(4096)
    {
        *(.data)
        *(.rodata*)
        *(.bss)
    }

}
