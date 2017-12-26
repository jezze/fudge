#include <abi.h>
#include <fudge.h>
#include "echo.h"

void write_keybuffer(unsigned int descriptor, char *key, void *data, unsigned int length)
{

    union echo_arg a[2];

    a[0].s = key;
    a[1].b.data = data;
    a[1].b.length = length;

    echo(descriptor, "%s: %b\n", a);

}

void write_keystring(unsigned int descriptor, char *key, char *value)
{

    union echo_arg a[2];

    a[0].s = key;
    a[1].s = value;

    echo(descriptor, "%s: %s\n", a);

}

void write_keyboolean(unsigned int descriptor, char *key, unsigned int value)
{

    union echo_arg a[2];

    a[0].s = key;
    a[1].s = (value) ? "true" : "false";

    echo(descriptor, "%s: %s\n", a);

}

void write_keydec(unsigned int descriptor, char *key, unsigned int value)
{

    union echo_arg a[2];

    a[0].s = key;
    a[1].u = value;

    echo(descriptor, "%s: %u\n", a);

}

void write_keyhex(unsigned int descriptor, char *key, unsigned int value, unsigned int padding)
{

    union echo_arg a[2];

    a[0].s = key;
    a[1].u = value;

    echo(descriptor, "%s: 0x%h\n", a);

}

