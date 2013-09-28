#define UART_BASE1                      0x03F8
#define UART_BASE2                      0x02F8
#define UART_BASE3                      0x03E8
#define UART_BASE4                      0x02E8
#define UART_IRQ1                       0x04
#define UART_IRQ2                       0x03
#define UART_DEVICE_TYPE                0x1000

struct uart_device
{

    struct base_device base;
    unsigned int port;

};

void uart_init_device(struct uart_device *device, unsigned int port, unsigned int irq, char *name);
