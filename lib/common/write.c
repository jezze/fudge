#include <abi.h>
#include <fudge.h>

void write_keyvalue(unsigned int descriptor, char *key, void *value, unsigned int count)
{

    file_writeall(descriptor, key, ascii_length(key));
    file_writeall(descriptor, ": ", 2);
    file_writeall(descriptor, value, count);
    file_writeall(descriptor, "\n", 1);

}

void write_keystring(unsigned int descriptor, char *key, char *value)
{

    write_keyvalue(descriptor, key, value, ascii_length(value));

}

void write_keyboolean(unsigned int descriptor, char *key, unsigned int value)
{

    write_keystring(descriptor, key, (value) ? "true" : "false");

}

void write_keydec(unsigned int descriptor, char *key, unsigned int value)
{

    char num[FUDGE_NSIZE];

    write_keyvalue(descriptor, key, num, ascii_wvalue(num, FUDGE_NSIZE, value, 10, 0));

}

void write_keyhex(unsigned int descriptor, char *key, unsigned int value, unsigned int padding)
{

    char num[FUDGE_NSIZE];
    unsigned int count = memory_write(num, FUDGE_NSIZE, "0x", 2, 0) + ascii_wzerovalue(num, FUDGE_NSIZE, value, 16, 2, padding);

    write_keyvalue(descriptor, key, num, count);

}

