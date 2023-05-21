#include <fudge.h>
#include <abi.h>
#include "strpool.h"

#define MAX_STRINGS                     512
#define STRINGDATA_SIZE                 0x4000

struct strindex
{

    unsigned int offset;
    unsigned int length;

};

static char strdata[STRINGDATA_SIZE];
static unsigned int strdataoffset;
static struct strindex strindex[MAX_STRINGS];

static unsigned int findslot(void)
{

    unsigned int i;

    for (i = 1; i < MAX_STRINGS; i++)
    {

        struct strindex *s = &strindex[i];

        if (s->offset == 0 && s->length == 0)
            return i;

    }

    return 0;

}

char *strpool_getstring(unsigned int index)
{

    struct strindex *s = &strindex[index];

    return strdata + s->offset;

}

unsigned int strpool_getcstringlength(unsigned int index)
{

    struct strindex *s = &strindex[index];

    return (s->length) ? s->length - 1 : 0;

}

static unsigned int savedata(unsigned int count, void *data)
{

    unsigned int slot = findslot();

    if (slot)
    {

        struct strindex *s = &strindex[slot];

        s->offset = strdataoffset;
        s->length = count;
        strdataoffset += buffer_write(strdata, STRINGDATA_SIZE, data, s->length, s->offset);

    }

    return slot;

}

static unsigned int freedata(unsigned int index)
{

    struct strindex *s = &strindex[index];
    unsigned int offset = s->offset;
    unsigned int length = s->length;
    unsigned int next = s->offset + s->length;
    unsigned int count = strdataoffset - next;
    unsigned int i;

    buffer_write(strdata, STRINGDATA_SIZE, strdata + next, count, offset);
    strdataoffset -= length;

    s->offset = 0;
    s->length = 0;

    for (i = 0; i < MAX_STRINGS; i++)
    {

        struct strindex *current = &strindex[i];

        if (current->offset > offset)
            current->offset -= length;

    }

    return 0;

}

unsigned int strpool_updatestring(unsigned int index, char *cstring)
{

    if (index)
        index = freedata(index);

    if (cstring)
        index = savedata(cstring_length_zero(cstring), cstring);

    return index;

}

