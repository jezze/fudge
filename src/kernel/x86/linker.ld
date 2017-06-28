ENTRY (init)

SECTIONS
{

    . = 0x00100000;

    .mboot :
    {
        src/kernel/x86/init.o (.data)
    }

    .text :
    {
        * (.text)
    }

    .data :
    {
        * (.data)
    }

}
