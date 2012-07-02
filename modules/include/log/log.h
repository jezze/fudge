#ifndef MODULES_LOG_H
#define MODULES_LOG_H

#define LOG_DRIVER_TYPE 0x1540

struct log_buffer
{

    char buffer[0x1000];
    unsigned int count;

};

struct log_driver
{

    struct modules_driver base;
    struct log_buffer buffer;

};

void log_write(unsigned int module, unsigned int count, char *buffer);
void log_driver_init(struct log_driver *driver);

#endif
