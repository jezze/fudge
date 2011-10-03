#ifndef KERNEL_LOG_H
#define KERNEL_LOG_H

#define LOG_TYPE_INFO    0
#define LOG_TYPE_ERROR   1
#define LOG_TYPE_WARNING 2

extern char *log_get();
extern void log_message(unsigned int type, char *message, void **args);
extern void log_init();

#endif

