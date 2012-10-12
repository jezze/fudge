#ifndef ARCH_UART_H
#define ARCH_UART_H

#define UART_REG_DATA           0x16000000
#define UART_REG_FLAG           0x16000018

#define UART_FLAG_FULL          (1 << 5)

void uart_putc(char c);
void uart_puts(const char *s);
void uart_setup();

#endif

