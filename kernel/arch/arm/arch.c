#define SERIAL_BASE 0x16000000
#define SERIAL_FLAG_REGISTER 0x18
#define SERIAL_BUFFER_FULL (1 << 5)

void screen_putc(char c)
{

    *((unsigned long*)0xC0000000) = c;

}

void putc(char c)
{

    while (*(volatile unsigned long*)(SERIAL_BASE + SERIAL_FLAG_REGISTER) & (SERIAL_BUFFER_FULL));

    *(volatile unsigned long*)SERIAL_BASE = c;

}

void puts(const char *str)
{

    while (*str) putc(*str++);

}

int arch_init(void)
{

    screen_putc('h');
    puts("Hello World!\n");

    return 0;

}

