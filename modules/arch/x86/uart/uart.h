#define UART_BASE1                      0x03F8
#define UART_BASE2                      0x02F8
#define UART_BASE3                      0x03E8
#define UART_BASE4                      0x02E8

#define UART_RBR                        0x0000
#define UART_THR                        0x0000
#define UART_DLL                        0x0000
#define UART_DLM                        0x0000
#define UART_IER                        0x0001
#define UART_IIR                        0x0002
#define UART_FCR                        0x0002
#define UART_LCR                        0x0003
#define UART_MCR                        0x0004
#define UART_LSR                        0x0005
#define UART_MSR                        0x0006
#define UART_SCR                        0x0007

#define UART_IRQ1                       0x04
#define UART_IRQ2                       0x03

#define UART_DEVICE_TYPE                0x1000

struct uart_device
{

    struct base_device base;
    unsigned int port;

};

struct uart_driver
{

    struct base_driver base;
    struct terminal_interface iterminal;
    struct circular_stream stream;

};

char uart_device_read(struct uart_device *device);
void uart_device_write(struct uart_device *device, char c);
void uart_init_device(struct uart_device *device, unsigned int port, unsigned int irq);
void uart_init_driver(struct uart_driver *driver);
