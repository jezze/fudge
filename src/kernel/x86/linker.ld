ENTRY (mboot_init)

SECTIONS
{

    . = 0x00100000;

    .text :
    {
        *(.mboot)
        *(.text)
    }

}
