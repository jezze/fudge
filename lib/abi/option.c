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

static struct option *findfree(void)
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

static unsigned int setvalue(struct option *option, char *value)
{

    unsigned int length = cstring_length(value);

    if (length >= OPTION_VALUESIZE)
        return 0;

    buffer_copy(option->value, value, length + 1);

    return 1;

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

unsigned int option_setstring(char *key, char *value)
{

    struct option *option = find(key);

    return (option) ? setvalue(option, value) : 0;

}

void option_add(char *key, char *value)
{

    struct option *option = findfree();

    if (option)
    {

        option->keyhash = djb_hash(cstring_length(key), key);

        setvalue(option, value);

    }

}

