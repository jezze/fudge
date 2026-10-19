ENTRY (_start)

SECTIONS
{

    . = 0x80200000;

    .text : ALIGN(4096)
    {
        *(.text.boot)
        *(.text)
    }

    .data : ALIGN(4096)
    {
        *(.data)
        *(.rodata*)
        *(.srodata*)
        *(.sdata*)
        *(.bss)
        *(.sbss*)
        *(COMMON)
    }

}
