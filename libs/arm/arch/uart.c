#include <kernel.h>
#include "reg.h"
#include "uart.h"

void uart_putc(char c)
{

    while (reg_read32(UART_REG_FLAG) & UART_FLAG_FULL);

    reg_write8(UART_REG_DATA, c);

}

void uart_puts(const char *s)
{

    while (*s)
        uart_putc(*s++);

}

void uart_setup()
{

}

