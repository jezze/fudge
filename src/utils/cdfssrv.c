#include <fudge.h>
#include <abi.h>
#include <disk.h>

#define NAMESIZE                        256
#define NAMES_PLAIN                     0
#define NAMES_JOLIET                    1
#define NAMES_ROCKRIDGE                 2

struct node
{

    unsigned int found;
    unsigned int extent;
    unsigned int size;
    unsigned int type;

};

struct cursor
{

    unsigned int extent;
    unsigned int size;
    unsigned int next;
    unsigned int id;
    struct node node;
    unsigned int length;
    char name[NAMESIZE];

};

struct span
{

    unsigned int sector;
    unsigned int sectors;
    unsigned int count;

};

static struct message_blockinfo blockinfo;
static unsigned int block;
static unsigned int partoffset;
static unsigned int root;
static unsigned int names;
static unsigned int suspskip;
static unsigned char cache[CDFS_SECTORSIZE];
static unsigned int cached;

static unsigned int sendblockrequest(unsigned int sector, unsigned int count, unsigned int target)
{

    struct message_blockrequest request;

    request.offset = partoffset + sector * CDFS_SECTORSIZE;
    request.count = count;
    request.target = target;

    if (channel_send(1, block, MESSAGE_TYPE_BLOCKREADREQUEST, sizeof (struct message_blockrequest), &request))
    {

        struct message_blockresponse response;

        return (channel_wait(1, block, MESSAGE_TYPE_BLOCKREADRESPONSE, sizeof (struct message_blockresponse), &response)) ? response.count : 0;

    }

    return 0;

}

static unsigned int readsector(unsigned int sector)
{

    if (cached != sector + 1)
    {

        if (sendblockrequest(sector, CDFS_SECTORSIZE, 0) != CDFS_SECTORSIZE)
            return 0;

        buffer_copy(cache, (void *)blockinfo.buffer, CDFS_SECTORSIZE);

        cached = sector + 1;

    }

    return 1;

}

static unsigned int checkentry(unsigned int offset)
{

    struct cdfs_entry *entry = (struct cdfs_entry *)&cache[offset];

    return offset + CDFS_ENTRYSIZE <= CDFS_SECTORSIZE && entry->length >= CDFS_ENTRYSIZE + entry->namelength && offset + entry->length <= CDFS_SECTORSIZE;

}

static struct node getentrynode(unsigned int offset)
{

    struct cdfs_entry *entry = (struct cdfs_entry *)&cache[offset];
    struct node node;

    node.found = 1;
    node.extent = cdfs_read32(entry->extent);
    node.size = cdfs_read32(entry->size);
    node.type = (entry->flags & CDFS_FLAG_DIRECTORY) ? RECORD_TYPE_DIRECTORY : RECORD_TYPE_NORMAL;

    return node;

}

static struct node getnode(unsigned int id)
{

    struct node node;

    if (id && readsector(id / CDFS_SECTORSIZE) && checkentry(id % CDFS_SECTORSIZE))
        return getentrynode(id % CDFS_SECTORSIZE);

    node.found = 0;
    node.extent = 0;
    node.size = 0;
    node.type = RECORD_TYPE_NORMAL;

    return node;

}

static void appendname(struct cursor *cursor, unsigned char c)
{

    if (cursor->length < NAMESIZE - 1)
    {

        cursor->name[cursor->length] = c;
        cursor->length++;

    }

}

static void getplainname(struct cursor *cursor, unsigned int offset, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count && cache[offset + i] != ';'; i++)
        appendname(cursor, (cache[offset + i] >= 'A' && cache[offset + i] <= 'Z') ? cache[offset + i] + ('a' - 'A') : cache[offset + i]);

    if (cursor->length > 1 && cursor->name[cursor->length - 1] == '.')
        cursor->length--;

}

static void getjolietname(struct cursor *cursor, unsigned int offset, unsigned int count)
{

    unsigned int i;

    for (i = 0; i + 1 < count; i += 2)
    {

        unsigned int codepoint = (cache[offset + i] << 8) | cache[offset + i + 1];

        if (codepoint == ';')
            break;

        if (codepoint >= 0xD800 && codepoint < 0xDC00 && i + 3 < count && cache[offset + i + 2] >= 0xDC && cache[offset + i + 2] < 0xE0)
        {

            codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (((cache[offset + i + 2] << 8) | cache[offset + i + 3]) - 0xDC00);
            i += 2;

        }

        else if (codepoint >= 0xD800 && codepoint < 0xE000)
        {

            codepoint = 0xFFFD;

        }

        cursor->length += utf8_write(cursor->name + cursor->length, NAMESIZE - 1 - cursor->length, codepoint);

    }

}

static unsigned int getrockridgename(struct cursor *cursor, unsigned int offset)
{

    struct cdfs_entry *entry = (struct cdfs_entry *)&cache[offset];
    unsigned int end = offset + entry->length;
    unsigned int position = offset + CDFS_ENTRYSIZE + entry->namelength + ((entry->namelength % 2) ? 0 : 1) + suspskip;

    while (position + 4 <= end && cache[position + 2] >= 4 && position + cache[position + 2] <= end)
    {

        unsigned int length = cache[position + 2];

        if (cache[position] == 'S' && cache[position + 1] == 'T')
            break;

        if (cache[position] == 'N' && cache[position + 1] == 'M')
        {

            unsigned int i;

            for (i = 5; i < length; i++)
                appendname(cursor, cache[position + i]);

        }

        position += length;

    }

    return cursor->length;

}

static void getname(struct cursor *cursor, unsigned int offset)
{

    struct cdfs_entry *entry = (struct cdfs_entry *)&cache[offset];
    unsigned int nameoffset = offset + CDFS_ENTRYSIZE;

    cursor->length = 0;

    if (entry->namelength == 1 && cache[nameoffset] < 2)
    {

        appendname(cursor, '.');

        if (cache[nameoffset] == 1)
            appendname(cursor, '.');

    }

    else if (names == NAMES_JOLIET)
    {

        getjolietname(cursor, nameoffset, entry->namelength);

    }

    else if (names != NAMES_ROCKRIDGE || !getrockridgename(cursor, offset))
    {

        getplainname(cursor, nameoffset, entry->namelength);

    }

}

static unsigned int nextentry(struct cursor *cursor)
{

    while (cursor->next < cursor->size)
    {

        unsigned int sector = cursor->extent + cursor->next / CDFS_SECTORSIZE;
        unsigned int offset = cursor->next % CDFS_SECTORSIZE;
        struct cdfs_entry *entry = (struct cdfs_entry *)&cache[offset];

        if (!readsector(sector))
            return 0;

        if (offset + CDFS_ENTRYSIZE > CDFS_SECTORSIZE || !entry->length)
        {

            cursor->next += CDFS_SECTORSIZE - offset;

            continue;

        }

        if (!checkentry(offset))
            return 0;

        cursor->next += entry->length;

        if (!(entry->flags & CDFS_FLAG_ASSOCIATED))
        {

            cursor->node = getentrynode(offset);
            cursor->id = (cursor->node.type == RECORD_TYPE_DIRECTORY) ? cursor->node.extent * CDFS_SECTORSIZE : sector * CDFS_SECTORSIZE + offset;

            getname(cursor, offset);

            return 1;

        }

    }

    return 0;

}

static void initcursor(struct cursor *cursor, struct node *directory, unsigned int offset)
{

    cursor->extent = directory->extent;
    cursor->size = directory->size;
    cursor->next = offset;
    cursor->length = 0;

}

static unsigned int findentry(unsigned int id, char *path, unsigned int length)
{

    struct node node = getnode(id);
    struct cursor cursor;

    if (node.type != RECORD_TYPE_DIRECTORY)
        return 0;

    initcursor(&cursor, &node, 0);

    while (nextentry(&cursor))
    {

        if (cursor.length == length && buffer_match(cursor.name, path, length))
            return cursor.id;

    }

    return 0;

}

static unsigned int walk(unsigned int id, char *path, unsigned int length)
{

    unsigned int offset = 0;

    while (offset < length)
    {

        unsigned int next = buffer_eachbyte(path, length, '/', offset);
        unsigned int count = (next ? next : length + 1) - offset - 1;

        if (count)
        {

            id = findentry(id, path + offset, count);

            if (!id)
                return 0;

        }

        offset += count + 1;

    }

    return id;

}

static unsigned int readdirectory(struct node *directory, unsigned int offset, unsigned int capacity, struct record *records)
{

    struct cursor cursor;
    unsigned int i = 0;

    initcursor(&cursor, directory, offset);

    while ((i + 1) * sizeof (struct record) <= capacity && nextentry(&cursor))
    {

        record_init(&records[i], cursor.id, cursor.node.type, cursor.node.size, cursor.next, cursor.length, cursor.name);

        i++;

    }

    return i * sizeof (struct record);

}

static struct span getspan(struct node *node, unsigned int offset, unsigned int count, unsigned int capacity)
{

    unsigned int skip = offset % CDFS_SECTORSIZE;
    unsigned int room = blockinfo.buffersize - blockinfo.buffersize % CDFS_SECTORSIZE - skip;
    struct span span;

    span.count = count;

    if (span.count > capacity)
        span.count = capacity;

    if (span.count > node->size - offset)
        span.count = node->size - offset;

    if (span.count > room)
        span.count = room;

    span.sector = node->extent + offset / CDFS_SECTORSIZE;
    span.sectors = (skip + span.count + CDFS_SECTORSIZE - 1) / CDFS_SECTORSIZE;

    return span;

}

static unsigned int readfile(struct node *node, unsigned int offset, unsigned int count, unsigned int capacity, void *data)
{

    if (offset < node->size)
    {

        struct span span = getspan(node, offset, count, capacity);

        if (sendblockrequest(span.sector, span.sectors * CDFS_SECTORSIZE, 0) == span.sectors * CDFS_SECTORSIZE)
            return buffer_write(data, capacity, (char *)blockinfo.buffer + offset % CDFS_SECTORSIZE, span.count, 0);

    }

    return 0;

}

static unsigned int sendfile(struct node *node, unsigned int offset, unsigned int count, unsigned int target)
{

    if (offset < node->size)
    {

        struct span span = getspan(node, offset, count, count);

        if (sendblockrequest(span.sector, span.sectors * CDFS_SECTORSIZE, target) == span.sectors * CDFS_SECTORSIZE)
            return span.count;

    }

    return 0;

}

static unsigned int stat(unsigned int id, struct record *record)
{

    struct node node = getnode(id);
    struct cursor cursor;

    if (!node.found)
        return 0;

    cursor.length = 0;

    /* getnode left the record's sector in the cache */
    if (node.type != RECORD_TYPE_DIRECTORY)
        getname(&cursor, id % CDFS_SECTORSIZE);

    record_init(record, id, node.type, node.size, 0, cursor.length, cursor.name);

    return sizeof (struct record);

}

static unsigned int isjoliet(struct cdfs_descriptor *descriptor)
{

    return descriptor->escape[0] == '%' && descriptor->escape[1] == '/' && (descriptor->escape[2] == '@' || descriptor->escape[2] == 'C' || descriptor->escape[2] == 'E');

}

static unsigned int hassusp(void)
{

    struct cdfs_entry *entry = (struct cdfs_entry *)cache;

    if (readsector(root / CDFS_SECTORSIZE) && checkentry(0) && entry->length >= CDFS_ENTRYSIZE + 8 && cache[CDFS_ENTRYSIZE + 1] == 'S' && cache[CDFS_ENTRYSIZE + 2] == 'P' && cache[CDFS_ENTRYSIZE + 5] == 0xBE && cache[CDFS_ENTRYSIZE + 6] == 0xEF)
    {

        suspskip = cache[CDFS_ENTRYSIZE + 7];

        return 1;

    }

    return 0;

}

static unsigned int findroot(void)
{

    struct cdfs_descriptor *descriptor = (struct cdfs_descriptor *)cache;
    unsigned int joliet = 0;
    unsigned int sector;

    for (sector = CDFS_DESCRIPTORSECTOR; sector < CDFS_DESCRIPTORSECTOR + CDFS_DESCRIPTORLIMIT && readsector(sector); sector++)
    {

        if (!cdfs_validate(descriptor) || descriptor->type == CDFS_TYPE_TERMINATOR)
            break;

        if (!checkentry(descriptor->root - cache))
            continue;

        if (descriptor->type == CDFS_TYPE_PRIMARY && !root)
            root = getentrynode(descriptor->root - cache).extent * CDFS_SECTORSIZE;

        if (descriptor->type == CDFS_TYPE_SUPPLEMENTARY && !joliet && isjoliet(descriptor))
            joliet = getentrynode(descriptor->root - cache).extent * CDFS_SECTORSIZE;

    }

    if (root && hassusp())
        names = NAMES_ROCKRIDGE;
    else if (joliet)
        names = NAMES_JOLIET;
    else
        names = NAMES_PLAIN;

    if (names == NAMES_JOLIET)
        root = joliet;

    return root;

}

static void onreadrequest(struct message *message)
{

    unsigned char data[MESSAGE_SIZE];
    struct message_readrequest *request = message_validate(MESSAGE_TYPE_READREQUEST, message->data, message->length);
    struct message_readresponse *response = (struct message_readresponse *)data;

    response->count = 0;
    response->buffer = 0;

    if (request)
    {

        struct node node = getnode(request->id);

        if (node.type == RECORD_TYPE_DIRECTORY)
        {

            response->count = readdirectory(&node, request->offset, MESSAGE_SIZE - sizeof (struct message_readresponse), (struct record *)(response + 1));

        }

        else
        {

            if (request->flags & READ_FLAG_BUFFER)
                response->count = sendfile(&node, request->offset, request->count, message->source);

            if (response->count)
                response->buffer = blockinfo.buffer + request->offset % CDFS_SECTORSIZE;
            else
                response->count = readfile(&node, request->offset, request->count, MESSAGE_SIZE - sizeof (struct message_readresponse), response + 1);

        }

    }

    channel_send(0, message->source, MESSAGE_TYPE_READRESPONSE, sizeof (struct message_readresponse) + ((response->buffer) ? 0 : response->count), data);

}

static void onstatrequest(struct message *message)
{

    unsigned char data[MESSAGE_SIZE];
    struct message_statrequest *request = message_validate(MESSAGE_TYPE_STATREQUEST, message->data, message->length);
    struct message_statresponse *response = (struct message_statresponse *)data;

    response->count = (request) ? stat(request->id, (struct record *)(response + 1)) : 0;

    channel_send(0, message->source, MESSAGE_TYPE_STATRESPONSE, sizeof (struct message_statresponse) + response->count, data);

}

static void onwalkrequest(struct message *message)
{

    struct message_walkrequest *request = message_validate(MESSAGE_TYPE_WALKREQUEST, message->data, message->length);
    struct message_walkresponse response;

    response.id = (request) ? walk((request->parent) ? request->parent : root, (char *)(request + 1), request->length) : 0;

    channel_send(0, message->source, MESSAGE_TYPE_WALKRESPONSE, sizeof (struct message_walkresponse), &response);

}

static void onmain(struct message *message)
{

    block = channel_lookup(option_getstring("block-service"));
    partoffset = option_getdecimal("partoffset");

    if (block)
    {

        channel_send(1, block, MESSAGE_TYPE_INFO, 0, 0);

        if (!channel_wait(1, block, MESSAGE_TYPE_BLOCKINFO, sizeof (struct message_blockinfo), &blockinfo))
            return;

        if (findroot())
        {

            if (call_announce(0, cstring_length(option_getstring("service")), option_getstring("service")) != MESSAGE_OK)
            {

                channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Could not announce service: %s\n", option_getstring("service"));

                return;

            }

            channel_hold(0);

        }

    }

}

void init(void)
{

    option_add("service", "cdfs");
    option_add("block-service", "block:0");
    option_add("partoffset", "0");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_READREQUEST, onreadrequest);
    channel_bind(MESSAGE_TYPE_STATREQUEST, onstatrequest);
    channel_bind(MESSAGE_TYPE_WALKREQUEST, onwalkrequest);

}

