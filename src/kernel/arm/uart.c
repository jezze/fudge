#include "reg.h"
#include "pic.h"
#include "uart.h"

#define UART_REG_DATA                   0x16000000
#define UART_REG_FLAG                   0x16000018

#define UART_FLAG_FULL                  (1 << 5)

void uart_putc(char c)
{

    while (reg_read32(UART_REG_FLAG) & UART_FLAG_FULL);

    reg_write8(UART_REG_DATA, c);

}

void uart_puts(char *s)
{

    while (*s)
        uart_putc(*s++);

}

void uart_irq(void)
{

    reg_write32(0x16000044, 1);
    uart_puts("UART0\n");

}

void uart_setup(void)
{

    pic_enableirq(PIC_IRQ_UART0);

}

