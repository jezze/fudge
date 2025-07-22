#include <fudge.h>
#include <hash.h>
#include "option.h"

static struct option options[OPTION_MAX];

static struct option *find(char *key)
{

    unsigned int keyhash = djb_hash(cstring_length(key), key);
    unsigned int i;

    for (i = 0; i < OPTION_MAX; i++)
    {

        struct option *option = &options[i];

        if (option->keyhash == keyhash)
            return option;

    }

    return 0;

}

static struct option *findfree(char *key)
{

    unsigned int i;

    for (i = 0; i < OPTION_MAX; i++)
    {

        struct option *option = &options[i];

        if (!option->keyhash)
            return option;

    }

    return 0;

}

int option_getdecimal(char *key)
{

    struct option *option = find(key);

    return (option) ? cstring_read_value(option->value, cstring_length(option->value), 10) : 0;

}

char *option_getstring(char *key)
{

    struct option *option = find(key);

    return (option) ? option->value : 0;

}

unsigned int option_setdecimal(char *key, int value)
{

    struct option *option = find(key);

    return (option) ? cstring_write_zero(option->value, 64, cstring_write_value(option->value, 64, value, 10, 0, 0)) : 0;

}

unsigned int option_setstring(char *key, char *value)
{

    struct option *option = find(key);

    return (option) ? cstring_write_zero(option->value, 64, cstring_write(option->value, 64, value, 0)) : 0;

}

void option_add(char *key, char *value)
{

    struct option *option = findfree(key);

    if (option)
        option_init(option, key, value);

}

void option_init(struct option *option, char *key, char *value)
{

    option->keyhash = djb_hash(cstring_length(key), key);

    cstring_write_zero(option->value, 64, cstring_write(option->value, 64, value, 0));

}

