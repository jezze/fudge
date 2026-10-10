ENTRY (start)

SECTIONS
{

    . = 0x80000000;

    .text : ALIGN(4096)
    {
        *(.init);
        *(.text);
    }

    .bss : ALIGN(4096)
    {
        PROVIDE(bss_start = .);
        *(.bss);
        . += 4096;
        PROVIDE(stack_top = .);
        . += 4096;
        PROVIDE(global_pointer = .);
        PROVIDE(bss_end = .);
    }

    .rodata : ALIGN(4096)
    {
        *(.rodata);
    }

    .data : ALIGN(4096)
    {
        *(.data);
    }

}

