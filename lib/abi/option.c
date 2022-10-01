#include <fudge.h>
#include "option.h"

static struct option options[32];

static struct option *find(char *key)
{

    unsigned int i;

    for (i = 0; options[i].key; i++)
    {

        struct option *option = &options[i];

        if (cstring_match(option->key, key))
            return option;

    }

    return 0;

}

static struct option *findfree(char *key)
{

    unsigned int i;

    for (i = 0; options[i].key; i++);

    return &options[i];

}

int option_getdecimal(char *key)
{

    struct option *option = find(key);

    return (option) ? cstring_readvalue(option->value, cstring_length(option->value), 10) : 0;

}

char *option_getstring(char *key)
{

    struct option *option = find(key);

    return (option) ? option->value : 0;

}

void option_set(char *key, char *value)
{

    struct option *option = find(key);

    if (option)
        cstring_copy(option->value, value);

}

void option_add(char *key, char *value)
{

    struct option *option = findfree(key);

    if (option)
        option_init(option, key, value);

}

void option_init(struct option *option, char *key, char *value)
{

    option->key = key;

    cstring_copy(option->value, value);

}

