#include <fudge.h>
#include <abi.h>
#include "kv.h"

unsigned int kv_match(struct event_wmevent *event, char *key)
{

    char *data = (char *)(event + 1);
    unsigned int klength = cstring_length(key);
    unsigned int start = 0;
    unsigned int i;

    for (i = 0; i < event->length; i++)
    {

        if (data[i] == '&')
            data[i] = '\0';

        if (data[i] == '\0')
        {

            if (buffer_match(data + start, key, klength))
                return start + klength;

            start = i + 1;

        }

    }

    return 0;

}

unsigned int kv_getvalue(struct event_wmevent *event, char *key, unsigned int base)
{

    unsigned int index = kv_match(event, key);
    char *data = (char *)(event + 1);

    return (index) ? cstring_read_value(data + index, cstring_length(data + index), base) : 0;

}

char *kv_getstring(struct event_wmevent *event, char *key)
{

    unsigned int index = kv_match(event, key);
    char *data = (char *)(event + 1);

    return (index) ? data + index : 0;

}

