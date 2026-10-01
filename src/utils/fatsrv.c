#include <fudge.h>
#include <abi.h>
#include <disk.h>

#define ROOT                            1
#define NAMESIZE                        264

static struct event_blockinfo blockinfo;
static unsigned int sectorsize;
static unsigned int clustersize;
static unsigned int fatstart;
static unsigned int datastart;
static unsigned int rootcluster;
static unsigned int cachefirst;
static unsigned int cacheindex;
static unsigned int cachecluster;

static void *readsector(unsigned int sector)
{

    unsigned int target = channel_lookup(option_getstring("block-service"));

    if (target)
    {

        struct event_blockrequest request;
        struct event_blockresponse response;

        request.offset = option_getdecimal("partoffset") + sector * sectorsize;
        request.count = sectorsize;

        channel_send(1, target, EVENT_BLOCKREADREQUEST, sizeof (struct event_blockrequest), &request);
        channel_wait(1, target, EVENT_BLOCKREADRESPONSE, sizeof (struct event_blockresponse), &response);

    }

    return (void *)blockinfo.buffer;

}

static unsigned int nextcluster(unsigned int cluster)
{

    unsigned int offset = cluster * 4;
    unsigned char *sector = readsector(fatstart + offset / sectorsize);
    unsigned int next;

    buffer_copy(&next, sector + offset % sectorsize, 4);

    return next & FAT_CLUSTER_MASK;

}

static unsigned int findsector(unsigned int first, unsigned int offset)
{

    unsigned int index = offset / clustersize;
    unsigned int cluster = first;
    unsigned int i = 0;

    if (cachefirst == first && cacheindex <= index)
    {

        cluster = cachecluster;
        i = cacheindex;

    }

    for (; i < index && cluster >= 2 && cluster < FAT_CLUSTER_END; i++)
        cluster = nextcluster(cluster);

    if (cluster < 2 || cluster >= FAT_CLUSTER_END)
        return 0;

    cachefirst = first;
    cacheindex = index;
    cachecluster = cluster;

    return datastart + (cluster - 2) * (clustersize / sectorsize) + (offset % clustersize) / sectorsize;

}

static unsigned int getcluster(struct fat_entry *entry)
{

    unsigned int cluster = (entry->clusterhigh << 16) | entry->clusterlow;

    return (cluster) ? cluster : rootcluster;

}

static unsigned int gettype(struct fat_entry *entry)
{

    return (entry->attributes & FAT_ATTRIBUTE_DIRECTORY) ? RECORD_TYPE_DIRECTORY : RECORD_TYPE_NORMAL;

}

static void getnode(unsigned int id, struct fat_entry *entry)
{

    if (id == ROOT)
    {

        buffer_clear(entry, sizeof (struct fat_entry));

        entry->attributes = FAT_ATTRIBUTE_DIRECTORY;
        entry->clusterhigh = rootcluster >> 16;
        entry->clusterlow = rootcluster & 0xFFFF;

    }

    else
    {

        unsigned char *sector = readsector(id / (sectorsize / sizeof (struct fat_entry)));

        buffer_copy(entry, sector + (id % (sectorsize / sizeof (struct fat_entry))) * sizeof (struct fat_entry), sizeof (struct fat_entry));

    }

}

static char lower(unsigned char c, unsigned int lowercase)
{

    return (lowercase && c >= 'A' && c <= 'Z') ? c + 32 : c;

}

static unsigned int getshortname(struct fat_entry *entry, char *name)
{

    unsigned int length = 0;
    unsigned int i;

    for (i = 0; i < 8 && entry->name[i] != ' '; i++)
        name[length++] = lower(entry->name[i], entry->lowercase & FAT_LOWERCASE_NAME);

    if (entry->name[8] != ' ')
    {

        name[length++] = '.';

        for (i = 8; i < 11 && entry->name[i] != ' '; i++)
            name[length++] = lower(entry->name[i], entry->lowercase & FAT_LOWERCASE_EXTENSION);

    }

    name[length] = '\0';

    return length;

}

static unsigned char getchecksum(struct fat_entry *entry)
{

    unsigned char checksum = 0;
    unsigned int i;

    for (i = 0; i < 11; i++)
        checksum = ((checksum & 1) << 7) + (checksum >> 1) + entry->name[i];

    return checksum;

}

static void addlongname(struct fat_longname *longname, char *name)
{

    unsigned short chars[13];
    unsigned int index = (longname->order & 0x1F) - 1;
    unsigned int i;

    if (index >= 20)
        return;

    buffer_copy(chars, longname->name1, 10);
    buffer_copy(chars + 5, longname->name2, 12);
    buffer_copy(chars + 11, longname->name3, 4);

    for (i = 0; i < 13; i++)
        name[index * 13 + i] = (chars[i] == 0x0000 || chars[i] == 0xFFFF) ? '\0' : (chars[i] < 0x80) ? chars[i] : '?';

    if (longname->order & 0x40)
        name[index * 13 + 13] = '\0';

}

static unsigned int nextentry(unsigned int first, unsigned int *offset, struct fat_entry *entry, char *name)
{

    unsigned int haslongname = 0;
    unsigned char checksum = 0;

    for (;;)
    {

        unsigned int sector = findsector(first, *offset);
        unsigned int slot = (*offset % sectorsize) / sizeof (struct fat_entry);

        if (!sector)
            return 0;

        buffer_copy(entry, (unsigned char *)readsector(sector) + slot * sizeof (struct fat_entry), sizeof (struct fat_entry));

        if (entry->name[0] == FAT_ENTRY_END)
            return 0;

        *offset += sizeof (struct fat_entry);

        if (entry->attributes == FAT_ATTRIBUTE_LONGNAME && entry->name[0] != FAT_ENTRY_DELETED)
        {

            struct fat_longname *longname = (struct fat_longname *)entry;

            addlongname(longname, name);

            checksum = longname->checksum;
            haslongname = 1;

            continue;

        }

        if (entry->name[0] == FAT_ENTRY_DELETED || (entry->attributes & FAT_ATTRIBUTE_VOLUME))
        {

            haslongname = 0;

            continue;

        }

        if (!haslongname || checksum != getchecksum(entry))
            getshortname(entry, name);

        return sector * (sectorsize / sizeof (struct fat_entry)) + slot;

    }

}

static unsigned int matchname(char *name, char *path, unsigned int length)
{

    unsigned int i;

    if (cstring_length(name) != length)
        return 0;

    for (i = 0; i < length; i++)
    {

        if (lower(name[i], 1) != lower(path[i], 1))
            return 0;

    }

    return 1;

}

static unsigned int findentry(unsigned int id, char *path, unsigned int length)
{

    struct fat_entry entry;
    char name[NAMESIZE];
    unsigned int offset = 0;
    unsigned int first;

    getnode(id, &entry);

    if (gettype(&entry) != RECORD_TYPE_DIRECTORY)
        return 0;

    first = getcluster(&entry);

    while ((id = nextentry(first, &offset, &entry, name)))
    {

        if (matchname(name, path, length))
            return id;

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

static unsigned int readdirectory(unsigned int first, unsigned int offset, unsigned int capacity, void *data)
{

    struct record *records = data;
    struct fat_entry entry;
    char name[NAMESIZE];
    unsigned int i = 0;
    unsigned int id;

    while ((i + 1) * sizeof (struct record) <= capacity && (id = nextentry(first, &offset, &entry, name)))
    {

        record_init(&records[i], id, gettype(&entry), entry.size, offset, cstring_length(name), name);

        i++;

    }

    return i * sizeof (struct record);

}

static unsigned int readfile(unsigned int first, unsigned int size, unsigned int offset, unsigned int count, unsigned int capacity, void *data)
{

    unsigned int sector;

    if (offset >= size)
        return 0;

    if (count > capacity)
        count = capacity;

    if (count > size - offset)
        count = size - offset;

    if (count > sectorsize - offset % sectorsize)
        count = sectorsize - offset % sectorsize;

    sector = findsector(first, offset);

    if (!sector)
        return 0;

    return buffer_write(data, capacity, (char *)readsector(sector) + offset % sectorsize, count, 0);

}

static unsigned int stat(unsigned int id, struct record *record)
{

    struct fat_entry entry;
    char name[NAMESIZE];
    unsigned int length = 0;

    getnode(id, &entry);

    if (id != ROOT)
        length = getshortname(&entry, name);

    record_init(record, id, gettype(&entry), entry.size, 0, length, name);

    return sizeof (struct record);

}

static void oncreaterequest(struct message *message)
{

    struct event_createresponse response;

    response.id = 0;

    channel_send(0, message->source, EVENT_CREATERESPONSE, sizeof (struct event_createresponse), &response);

}

static void onreadrequest(struct message *message)
{

    unsigned char data[MESSAGE_SIZE];
    struct event_readrequest *request = message->data;
    struct event_readresponse *response = (struct event_readresponse *)data;
    struct fat_entry entry;

    getnode(request->id, &entry);

    if (gettype(&entry) == RECORD_TYPE_DIRECTORY)
        response->count = readdirectory(getcluster(&entry), request->offset, MESSAGE_SIZE - sizeof (struct event_readresponse), response + 1);
    else
        response->count = readfile(getcluster(&entry), entry.size, request->offset, request->count, MESSAGE_SIZE - sizeof (struct event_readresponse), response + 1);

    channel_send(0, message->source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + response->count, data);

}

static void onremoverequest(struct message *message)
{

    struct event_removeresponse response;

    response.status = 0;

    channel_send(0, message->source, EVENT_REMOVERESPONSE, sizeof (struct event_removeresponse), &response);

}

static void onstatrequest(struct message *message)
{

    unsigned char data[MESSAGE_SIZE];
    struct event_statrequest *request = message->data;
    struct event_statresponse *response = (struct event_statresponse *)data;

    response->count = stat(request->id, (struct record *)(response + 1));

    channel_send(0, message->source, EVENT_STATRESPONSE, sizeof (struct event_statresponse) + response->count, data);

}

static void onwalkrequest(struct message *message)
{

    struct event_walkrequest *request = message->data;
    struct event_walkresponse response;

    response.id = walk((request->parent) ? request->parent : ROOT, (char *)(request + 1), request->length);

    channel_send(0, message->source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

}

static void onwriterequest(struct message *message)
{

    struct event_writeresponse response;

    response.count = 0;

    channel_send(0, message->source, EVENT_WRITERESPONSE, sizeof (struct event_writeresponse), &response);

}

static void onmain(struct message *message)
{

    unsigned int block = channel_lookup(option_getstring("block-service"));

    if (block)
    {

        struct fat fat;

        channel_send(1, block, EVENT_INFO, 0, 0);
        channel_wait(1, block, EVENT_BLOCKINFO, sizeof (struct event_blockinfo), &blockinfo);

        sectorsize = 512;

        buffer_copy(&fat, readsector(0), sizeof (struct fat));

        if (fat_validate(&fat))
        {

            struct fat32 *fat32 = (struct fat32 *)fat.extended_section;

            sectorsize = fat.bytes_per_sector;
            clustersize = sectorsize * fat.sectors_per_cluster;
            fatstart = fat.reserved_sector_count;
            datastart = fatstart + fat.table_count * fat32->table_size_32;
            rootcluster = fat32->root_cluster;

            channel_hold();
            call_announce(0, cstring_length(option_getstring("service")), option_getstring("service"));

        }

    }

}

void init(void)
{

    option_add("service", "fat");
    option_add("block-service", "block");
    option_add("partoffset", "0");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_CREATEREQUEST, oncreaterequest);
    channel_bind(EVENT_READREQUEST, onreadrequest);
    channel_bind(EVENT_REMOVEREQUEST, onremoverequest);
    channel_bind(EVENT_STATREQUEST, onstatrequest);
    channel_bind(EVENT_WALKREQUEST, onwalkrequest);
    channel_bind(EVENT_WRITEREQUEST, onwriterequest);

}

