#ifndef MODULES_UART_H
#define MODULES_UART_H

#define UART_BASE1                      0x03F8
#define UART_BASE2                      0x02F8
#define UART_BASE3                      0x03E8
#define UART_BASE4                      0x02E8
#define UART_RBR                        0
#define UART_THR                        0
#define UART_DLL                        0
#define UART_DLM                        0
#define UART_IER                        1
#define UART_IIR                        2
#define UART_FCR                        2
#define UART_LCR                        3
#define UART_MCR                        4
#define UART_LSR                        5
#define UART_MSR                        6
#define UART_SCR                        7

#define UART_IRQ1                       0x04
#define UART_IRQ2                       0x03

#define UART_DEVICE_TYPE                0x0002
#define UART_DRIVER_TYPE                0x0232

struct uart_device
{

    struct base_device base;
    unsigned int port;
    unsigned int irq;
    char (*read)(struct uart_device *self);
    void (*write)(struct uart_device *self, char c);

};

struct uart_buffer
{

    char buffer[256];
    unsigned int size;
    unsigned int head;
    unsigned int tail;
    unsigned int (*getc)(struct uart_buffer *self, char *buffer);
    unsigned int (*putc)(struct uart_buffer *self, char *buffer);

};

struct uart_driver
{

    struct base_driver base;
    struct uart_buffer buffer;

};

void uart_init_device(struct uart_device *device, unsigned int port, unsigned int irq);
void uart_init_driver(struct uart_driver *driver);

#endif

