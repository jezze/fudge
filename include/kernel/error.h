#ifndef KERNEL_ERROR_H
#define KERNEL_ERROR_H

extern void error_assert(unsigned int condition, char *message, char *file, unsigned int line);
extern void error_panic(char *message, char *file, unsigned int line);

#endif

