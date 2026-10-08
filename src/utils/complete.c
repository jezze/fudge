#include <fudge.h>
#include <abi.h>

#define INPUTSIZE                       256
#define BINPATH                         "initrd:bin"

static char input[INPUTSIZE];
static unsigned int inputcount;
static char common[RECORD_NAMESIZE];
static unsigned int commoncount;
static unsigned int isdirectory;
static unsigned int nmatches;
static char list[MESSAGE_SIZE];
static unsigned int listcount;

static unsigned int isblankchar(char c)
{

    return (c == ' ' || c == '\t');

}

static unsigned int isspecialchar(char c)
{

    switch (c)
    {

    case ' ':
    case '\t':
    case '|':
    case ';':
    case '&':
    case '<':
    case '>':
    case '\n':
        return 1;

    }

    return 0;

}

static void addmatch(struct record *record)
{

    unsigned int length = (record->length < RECORD_NAMESIZE) ? record->length : RECORD_NAMESIZE;

    if (nmatches)
    {

        unsigned int i;

        for (i = 0; i < commoncount && i < length && common[i] == record->name[i]; i++);

        commoncount = i;

    }

    else
    {

        buffer_copy(common, record->name, length);

        commoncount = length;
        isdirectory = (record->type == RECORD_TYPE_DIRECTORY);

    }

    if (record->type == RECORD_TYPE_DIRECTORY)
        listcount += cstring_write_fmt(list, MESSAGE_SIZE, listcount, "%w/\n", record->name, &length);
    else
        listcount += cstring_write_fmt(list, MESSAGE_SIZE, listcount, "%w\n", record->name, &length);

    nmatches++;

}

static void scandirectory(char *directory, char *prefix, unsigned int prefixcount)
{

    unsigned int target = fs_auth(directory);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, directory);

        if (id)
        {

            unsigned char data[MESSAGE_SIZE];
            unsigned int count;
            unsigned int offset = 0;

            while ((count = fs_read(1, target, id, data, MESSAGE_SIZE, offset)))
            {

                unsigned int i;

                for (i = 0; i < count; i += sizeof (struct record))
                {

                    struct record *record = (struct record *)(data + i);

                    if (record->length >= prefixcount && buffer_match(record->name, prefix, prefixcount))
                        addmatch(record);

                    offset = record->offset;

                }

            }

        }

    }

}

static void ondata(struct message *message)
{

    inputcount += buffer_write(input, INPUTSIZE, message->data, message->length, inputcount);

}

static void onterm(struct message *message)
{

    char directory[INPUTSIZE * 2];
    char output[INPUTSIZE];
    unsigned int count = 0;
    unsigned int start;
    unsigned int split;
    unsigned int prefixcount;
    unsigned int dircount;
    unsigned int command;
    unsigned int i;

    for (start = inputcount; start > 0 && !isspecialchar(input[start - 1]); start--);
    for (i = start; i > 0 && isblankchar(input[i - 1]); i--);
    for (split = inputcount; split > start && input[split - 1] != '/' && input[split - 1] != ':'; split--);

    command = (i == 0 || input[i - 1] == '|' || input[i - 1] == ';');
    prefixcount = inputcount - split;
    dircount = split - start;

    if (dircount)
    {

        char path[INPUTSIZE];

        cstring_write_fmt(path, INPUTSIZE, 0, "%w\\0", input + start, &dircount);
        fs_absolute(directory, INPUTSIZE * 2, option_getstring("pwd"), path);

    }

    else
    {

        cstring_write_fmt(directory, INPUTSIZE * 2, 0, "%s\\0", (command) ? BINPATH : option_getstring("pwd"));

    }

    scandirectory(directory, input + split, prefixcount);

    if (nmatches == 1)
    {

        count += buffer_write(output, INPUTSIZE, common + prefixcount, commoncount - prefixcount, count);
        count += buffer_write(output, INPUTSIZE, (isdirectory) ? "/" : " ", 1, count);

    }

    else if (commoncount > prefixcount)
    {

        count += buffer_write(output, INPUTSIZE, common + prefixcount, commoncount - prefixcount, count);

    }

    count += buffer_write(output, INPUTSIZE, "\n", 1, count);

    channel_send(0, message->source, EVENT_DATA, count, output);

    if (nmatches > 1 && commoncount == prefixcount)
        channel_send(0, message->source, EVENT_DATA, listcount, list);

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_TERM, onterm);

}

