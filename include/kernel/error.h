#ifndef KERNEL_ERROR_H
#define KERNEL_ERROR_H

#define ERROR_REGISTER_SLOTS 8

struct error_status
{

    unsigned int registers[ERROR_REGISTER_SLOTS];
    char *message;
    char *file;
    unsigned int line;

};

void error_assert(unsigned int condition, char *message, char *file, unsigned int line);
void error_panic(char *message, char *file, unsigned int line);
void error_register(unsigned int index, unsigned int value);

#endif

