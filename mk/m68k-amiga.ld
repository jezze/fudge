ENTRY (_start)

SECTIONS
{

    . = 0x10100000;

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
        *(COMMON)
    }

}
