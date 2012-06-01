#ifndef MODULES_LOG_H
#define MODULES_LOG_H

#define LOG_DRIVER_TYPE 0x1540

struct log_driver
{

    struct modules_driver base;

};

void log_driver_init(struct log_driver *driver);

#endif
