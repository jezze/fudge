#include <abi.h>
#include <fudge.h>

void write_keyvalue(char *key, void *value, unsigned int count)
{

    file_writeall(CALL_PO, key, ascii_length(key));
    file_writeall(CALL_PO, ": ", 2);
    file_writeall(CALL_PO, value, count);
    file_writeall(CALL_PO, "\n", 1);

}

void write_string(char *key, char *value)
{

    write_keyvalue(key, value, ascii_length(value));

}

void write_boolean(char *key, unsigned int value)
{

    write_string(key, (value) ? "true" : "false");

}

void write_dec(char *key, unsigned int value)
{

    char num[FUDGE_NSIZE];
    unsigned int count = ascii_wvalue(num, FUDGE_NSIZE, value, 10, 0);

    write_keyvalue(key, num, count);

}

void write_hex(char *key, unsigned int value)
{

    char num[FUDGE_NSIZE];
    unsigned int count = memory_write(num, FUDGE_NSIZE, "0x", 2, 0) + ascii_wvalue(num, FUDGE_NSIZE, value, 16, 2);

    write_keyvalue(key, num, count);

}

