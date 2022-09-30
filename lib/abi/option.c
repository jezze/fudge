#include <fudge.h>
#include "option.h"

static struct option *find(struct option *options, char *key)
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

static struct option *findfree(struct option *options, char *key)
{

    unsigned int i;

    for (i = 0; options[i].key; i++);

    return &options[i];

}

int option_getdecimal(struct option *options, char *key)
{

    struct option *option = find(options, key);

    return (option) ? cstring_readvalue(option->value, cstring_length(option->value), 10) : 0;

}

char *option_getstring(struct option *options, char *key)
{

    struct option *option = find(options, key);

    return (option) ? option->value : 0;

}

void option_set(struct option *options, char *key, char *value)
{

    struct option *option = find(options, key);

    if (option)
        cstring_copy(option->value, value);

}

void option_add(struct option *options, char *key, char *value)
{

    struct option *option = findfree(options, key);

    if (option)
        option_init(option, key, value);

}

void option_init(struct option *option, char *key, char *value)
{

    option->key = key;

    cstring_copy(option->value, value);

}

