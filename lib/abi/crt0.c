#include <fudge.h>
#include <abi.h>

extern void init(void);

static void onoption(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int i;
    char *key;

    for (i = 0; (key = buffer_tindex(mdata, msize, '\0', i)); i += 2)
    {

        char *value = key + cstring_length_zero(key);

        if (!option_set(key, value))
            channel_send_fmt1(0, source, EVENT_ERROR, "Unrecognized option: %s\n", key);

    }

}

static void onstatus(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int i;

    for (i = 0; i < OPTION_MAX; i++)
    {

        struct option *option = option_get(i);

        if (option)
            channel_send_fmt2(0, source, EVENT_DATA, "%s:%s\n", option->key, option->value);

    }

}

void lookup(char *key)
{

    char id[32];

    channel_send_fmt1(0, option_getdecimal("env"), EVENT_QUERYREQUEST, "get\\0%s\\0", key);
    channel_wait_buffer(0, option_getdecimal("env"), EVENT_QUERYRESPONSE, 32, id);
    option_set(key, id);

}

void lookup2(char *key, char *name, unsigned int index, unsigned int inode)
{

    option_setdecimal(key, call_find(cstring_length(name), name, index, inode));

}

void panic(unsigned int source, char *file, unsigned int line)
{

    channel_send_fmt2(0, source, EVENT_ERROR, "Process panic! File %s on line %u\n", file, &line);
    channel_close();
    call_despawn();

}

void main(void)
{

    option_add("env", "");
    option_add("pwd", "");
    channel_open();
    channel_bind(EVENT_OPTION, onoption);
    channel_bind(EVENT_STATUS, onstatus);
    init();

    while (channel_process(0));

}

