unsigned char *uart = (unsigned char *)0x10000000;

void putchar(char c)
{

    while (*(uart++))
        *uart = c;

}

void print(const char *str)
{

    while (*str != '\0')
    {

        putchar(*str);

        str++;

    }

}

void kmain(void)
{

    print("Hello world!\r\n");

    while (1)
    {

        putchar(*uart);

    }

}

