#include <fudge.h>
#include <hash.h>
#include "strpool.h"
#include "record.h"

#define RECORD_COUNT                    1024
#define RECORD_FIELDS                   8

struct dataentry
{

    unsigned int source;
    unsigned int set;
    unsigned int key;
    unsigned int hash;
    unsigned int selected;
    unsigned int anchor;
    unsigned int names[RECORD_FIELDS];
    unsigned int values[RECORD_FIELDS];

};

static struct dataentry records[RECORD_COUNT];
static unsigned int nrecords;

static unsigned int ismatch(struct dataentry *record, unsigned int source, char *set)
{

    return record->source == source && cstring_match(strpool_getstring(record->set), set);

}

static void clearrecord(struct dataentry *record)
{

    unsigned int i;

    record->set = strpool_updatestring(record->set, 0);
    record->key = strpool_updatestring(record->key, 0);

    for (i = 0; i < RECORD_FIELDS; i++)
    {

        record->names[i] = strpool_updatestring(record->names[i], 0);
        record->values[i] = strpool_updatestring(record->values[i], 0);

    }

}

static void copyrecord(struct dataentry *out, struct dataentry *in)
{

    unsigned int i;

    out->source = in->source;
    out->set = in->set;
    out->key = in->key;
    out->hash = in->hash;
    out->selected = in->selected;
    out->anchor = in->anchor;

    for (i = 0; i < RECORD_FIELDS; i++)
    {

        out->names[i] = in->names[i];
        out->values[i] = in->values[i];

    }

}

static void removeat(unsigned int index)
{

    unsigned int i;

    clearrecord(&records[index]);

    for (i = index; i + 1 < nrecords; i++)
        copyrecord(&records[i], &records[i + 1]);

    nrecords--;

}

struct dataentry *record_find(unsigned int source, char *set, char *key)
{

    unsigned int hash = djb_hash(cstring_length(key), key);
    unsigned int i;

    for (i = 0; i < nrecords; i++)
    {

        struct dataentry *record = &records[i];

        if (record->hash == hash && ismatch(record, source, set) && cstring_match(strpool_getstring(record->key), key))
            return record;

    }

    return 0;

}

struct dataentry *record_create(unsigned int source, char *set, char *key)
{

    struct dataentry *record;
    unsigned int i;

    if (nrecords >= RECORD_COUNT)
        return 0;

    record = &records[nrecords];
    record->source = source;
    record->set = strpool_updatestring(0, set);
    record->key = strpool_updatestring(0, key);
    record->hash = djb_hash(cstring_length(key), key);
    record->selected = 0;
    record->anchor = 0;

    for (i = 0; i < RECORD_FIELDS; i++)
    {

        record->names[i] = 0;
        record->values[i] = 0;

    }

    if (!record->set || !record->key)
    {

        clearrecord(record);

        return 0;

    }

    nrecords++;

    return record;

}

struct dataentry *record_next(struct dataentry *current, unsigned int source, char *set)
{

    unsigned int i;

    for (i = (current) ? current - records + 1 : 0; i < nrecords; i++)
    {

        struct dataentry *record = &records[i];

        if (ismatch(record, source, set))
            return record;

    }

    return 0;

}

struct dataentry *record_at(unsigned int source, char *set, unsigned int index)
{

    struct dataentry *record = 0;

    while ((record = record_next(record, source, set)) && index)
        index--;

    return record;

}

unsigned int record_count(unsigned int source, char *set)
{

    struct dataentry *record = 0;
    unsigned int count = 0;

    while ((record = record_next(record, source, set)))
        count++;

    return count;

}

unsigned int record_isselected(struct dataentry *record)
{

    return record->selected;

}

void record_select(struct dataentry *record, unsigned int selected)
{

    record->selected = selected;

}

unsigned int record_isanchor(struct dataentry *record)
{

    return record->anchor;

}

void record_setanchor(struct dataentry *record, unsigned int source, char *set)
{

    struct dataentry *current = 0;

    while ((current = record_next(current, source, set)))
        current->anchor = (current == record);

}

char *record_getkey(struct dataentry *record)
{

    return strpool_getstring(record->key);

}

char *record_getfield(struct dataentry *record, char *name, unsigned int length)
{

    unsigned int i;

    if (length == 3 && buffer_match(name, "key", 3))
        return strpool_getstring(record->key);

    for (i = 0; i < RECORD_FIELDS; i++)
    {

        if (record->names[i] && strpool_getcstringlength(record->names[i]) == length && buffer_match(strpool_getstring(record->names[i]), name, length))
            return strpool_getstring(record->values[i]);

    }

    return "";

}

void record_setfield(struct dataentry *record, char *name, char *value)
{

    unsigned int i;

    for (i = 0; i < RECORD_FIELDS; i++)
    {

        if (record->names[i] && cstring_match(strpool_getstring(record->names[i]), name))
        {

            record->values[i] = strpool_updatestring(record->values[i], value);

            return;

        }

    }

    for (i = 0; i < RECORD_FIELDS; i++)
    {

        if (!record->names[i])
        {

            record->names[i] = strpool_updatestring(0, name);
            record->values[i] = strpool_updatestring(0, value);

            return;

        }

    }

}

void record_remove(unsigned int source, char *set, char *key)
{

    unsigned int hash = (key) ? djb_hash(cstring_length(key), key) : 0;
    unsigned int i = 0;

    while (i < nrecords)
    {

        struct dataentry *record = &records[i];

        if (ismatch(record, source, set) && (!key || (record->hash == hash && cstring_match(strpool_getstring(record->key), key))))
            removeat(i);
        else
            i++;

    }

}

void record_removesource(unsigned int source)
{

    unsigned int i = 0;

    while (i < nrecords)
    {

        if (records[i].source == source)
            removeat(i);
        else
            i++;

    }

}
