ENTRY (mboot_init)

SECTIONS
{

    . = 0x00100000;

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
