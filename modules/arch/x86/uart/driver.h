struct uart_driver_stream
{

    char buffer[512];
    unsigned int head;
    unsigned int tail;

};

struct uart_driver
{

    struct base_driver base;
    struct base_terminal_interface iterminal;
    struct uart_driver_stream stream;

};

void uart_init_driver(struct uart_driver *driver);
