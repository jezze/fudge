ENTRY (mboot_init)

SECTIONS
{

    . = 0x00100000;

    .mboot :
    {
        src/kernel/x86/mboot/init.o(.text.mboot)
    }

    .text :
    {
        *(.text)
    }

    .data :
    {
        *(.data)
        *(.rodata*)
        *(.bss)
    }

}
