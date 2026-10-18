#include <fudge.h>
#include <abi.h>
#include <disk.h>

#define ROOT                            1
#define NAMESIZE                        264

static struct event_blockinfo blockinfo;
static unsigned int block;
static unsigned int partoffset;
static unsigned int sectorsize;
static unsigned int clustersize;
static unsigned int fatstart;
static unsigned int datastart;
static unsigned int rootcluster;
static unsigned int rootstart;
static unsigned int rootsectors;
static unsigned int fattype;
static unsigned int fatcount;
static unsigned int fatsize;
static unsigned int clustercount;
static unsigned int fsinfo;
static unsigned int cachefirst;
static unsigned int cacheindex;
static unsigned int cachecluster;
static unsigned int clock;

static void sendblockrequest(unsigned int event, unsigned int reply, unsigned int sector)
{

    struct event_blockrequest request;
    struct event_blockresponse response;

    request.offset = partoffset + sector * sectorsize;
    request.count = sectorsize;

    channel_send(1, block, event, sizeof (struct event_blockrequest), &request);
    channel_wait(1, block, reply, sizeof (struct event_blockresponse), &response);

}

static void *readsector(unsigned int sector)
{

    sendblockrequest(EVENT_BLOCKREADREQUEST, EVENT_BLOCKREADRESPONSE, sector);

    return (void *)blockinfo.buffer;

}

static void writesector(unsigned int sector)
{

    sendblockrequest(EVENT_BLOCKWRITEREQUEST, EVENT_BLOCKWRITERESPONSE, sector);

}

static unsigned char getfatbyte(unsigned int copy, unsigned int offset)
{

    unsigned char *data = readsector(fatstart + copy * fatsize + offset / sectorsize);

    return data[offset % sectorsize];

}

static void setfatbyte(unsigned int copy, unsigned int offset, unsigned char value)
{

    unsigned int sector = fatstart + copy * fatsize + offset / sectorsize;
    unsigned char *data = readsector(sector);

    data[offset % sectorsize] = value;

    writesector(sector);

}

static unsigned int nextcluster(unsigned int cluster)
{

    unsigned int next;

    switch (fattype)
    {

    case 12:
        next = getfatbyte(0, cluster + cluster / 2) | (getfatbyte(0, cluster + cluster / 2 + 1) << 8);
        next = (cluster & 1) ? next >> 4 : next & 0xFFF;

        return (next >= 0xFF8) ? FAT_CLUSTER_MASK : next;

    case 16:
        next = getfatbyte(0, cluster * 2) | (getfatbyte(0, cluster * 2 + 1) << 8);

        return (next >= 0xFFF8) ? FAT_CLUSTER_MASK : next;

    default:
        buffer_copy(&next, (unsigned char *)readsector(fatstart + cluster * 4 / sectorsize) + cluster * 4 % sectorsize, 4);

        return next & FAT_CLUSTER_MASK;

    }

}

static unsigned int clustersector(unsigned int cluster)
{

    return datastart + (cluster - 2) * (clustersize / sectorsize);

}

static void setcluster(unsigned int cluster, unsigned int value)
{

    unsigned int i;

    for (i = 0; i < fatcount; i++)
    {

        switch (fattype)
        {

        case 12:
            {

                unsigned int offset = cluster + cluster / 2;
                unsigned int old = getfatbyte(i, offset) | (getfatbyte(i, offset + 1) << 8);
                unsigned int new = (cluster & 1) ? (old & 0x000F) | ((value & 0xFFF) << 4) : (old & 0xF000) | (value & 0xFFF);

                setfatbyte(i, offset, new);
                setfatbyte(i, offset + 1, new >> 8);

            }

            break;

        case 16:
            setfatbyte(i, cluster * 2, value);
            setfatbyte(i, cluster * 2 + 1, value >> 8);

            break;

        default:
            {

                unsigned int sector = fatstart + i * fatsize + cluster * 4 / sectorsize;
                unsigned char *data = readsector(sector);
                unsigned int old;
                unsigned int new;

                buffer_copy(&old, data + cluster * 4 % sectorsize, 4);

                new = (old & ~FAT_CLUSTER_MASK) | (value & FAT_CLUSTER_MASK);

                buffer_copy(data + cluster * 4 % sectorsize, &new, 4);
                writesector(sector);

            }

            break;

        }

    }

}

static void addfree(unsigned int count, unsigned int used)
{

    unsigned char *data;
    unsigned int free;

    if (!fsinfo)
        return;

    data = readsector(fsinfo);

    buffer_copy(&free, data + 488, 4);

    if (free == 0xFFFFFFFF)
        return;

    free = free + count - used;

    buffer_copy(data + 488, &free, 4);
    writesector(fsinfo);

}

static unsigned int allocate(void)
{

    unsigned int perSector = sectorsize / 4;
    unsigned int *table = 0;
    unsigned short *table16 = 0;
    unsigned int cluster;

    for (cluster = 2; cluster < clustercount; cluster++)
    {

        unsigned int used;

        if (fattype == 32)
        {

            if (!table || cluster % perSector == 0)
                table = readsector(fatstart + cluster / perSector);

            used = table[cluster % perSector] & FAT_CLUSTER_MASK;

        }

        else if (fattype == 16)
        {

            if (!table16 || cluster % (sectorsize / 2) == 0)
                table16 = readsector(fatstart + cluster / (sectorsize / 2));

            used = table16[cluster % (sectorsize / 2)];

        }

        else
        {

            used = nextcluster(cluster);

        }

        if (!used)
        {

            unsigned int i;

            setcluster(cluster, FAT_CLUSTER_MASK);
            addfree(0, 1);

            for (i = 0; i < clustersize / sectorsize; i++)
            {

                buffer_clear((void *)blockinfo.buffer, sectorsize);
                writesector(clustersector(cluster) + i);

            }

            return cluster;

        }

    }

    return 0;

}

static unsigned int findcluster(unsigned int first, unsigned int index, unsigned int grow)
{

    unsigned int cluster = first;
    unsigned int i = 0;

    if (cachefirst == first && cacheindex <= index)
    {

        cluster = cachecluster;
        i = cacheindex;

    }

    if (cluster < 2 || cluster >= FAT_CLUSTER_END)
        return 0;

    for (; i < index; i++)
    {

        unsigned int next = nextcluster(cluster);

        if (next < 2 || next >= FAT_CLUSTER_END)
        {

            if (!grow || !(next = allocate()))
                return 0;

            setcluster(cluster, next);

        }

        cluster = next;
        cachefirst = first;
        cacheindex = i + 1;
        cachecluster = cluster;

    }

    cachefirst = first;
    cacheindex = index;
    cachecluster = cluster;

    return cluster;

}

static unsigned int findsector(unsigned int first, unsigned int offset)
{

    unsigned int cluster;

    if (!first)
        return (fattype != 32 && offset < rootsectors * sectorsize) ? rootstart + offset / sectorsize : 0;

    cluster = findcluster(first, offset / clustersize, 0);

    return (cluster) ? clustersector(cluster) + (offset % clustersize) / sectorsize : 0;

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

static void putnode(unsigned int id, struct fat_entry *entry)
{

    unsigned int perSector = sectorsize / sizeof (struct fat_entry);
    unsigned char *sector = readsector(id / perSector);

    buffer_copy(sector + (id % perSector) * sizeof (struct fat_entry), entry, sizeof (struct fat_entry));
    writesector(id / perSector);

}

static unsigned int getnow(unsigned short *date, unsigned short *time)
{

    struct event_clockinfo clockinfo;

    if (!clock)
        return 0;

    channel_send(1, clock, EVENT_INFO, 0, 0);

    if (!channel_wait(1, clock, EVENT_CLOCKINFO, sizeof (struct event_clockinfo), &clockinfo) || clockinfo.year < 1980)
        return 0;

    *date = ((clockinfo.year - 1980) << 9) | (clockinfo.month << 5) | clockinfo.day;
    *time = (clockinfo.hours << 11) | (clockinfo.minutes << 5) | (clockinfo.seconds / 2);

    return 1;

}

static void setcreated(struct fat_entry *entry)
{

    unsigned short date;
    unsigned short time;

    if (!getnow(&date, &time))
        return;

    entry->createdate = date;
    entry->createtime = time;
    entry->modifydate = date;
    entry->modifytime = time;
    entry->accessdate = date;

}

static unsigned int setmodified(struct fat_entry *entry)
{

    unsigned short date;
    unsigned short time;

    if (!getnow(&date, &time) || (entry->modifydate == date && entry->modifytime == time))
        return 0;

    entry->modifydate = date;
    entry->modifytime = time;
    entry->accessdate = date;

    return 1;

}

static unsigned int isshortchar(unsigned char c)
{

    if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
        return 1;

    switch (c)
    {

    case '_':
    case '-':
    case '~':
    case '!':
    case '#':
    case '$':
    case '%':
    case '&':
    case '(':
    case ')':
    case '@':
        return 1;

    }

    return 0;

}

static unsigned int makeshortname(struct fat_entry *entry, char *name, unsigned int length)
{

    unsigned int cases[2];
    unsigned int dot = length;
    unsigned int i;

    for (i = 0; i < length; i++)
    {

        if (name[i] == '.')
            dot = i;

    }

    if (!dot || dot > 8 || dot == length - 1 || length - dot > 4)
        return 0;

    cases[0] = 0;
    cases[1] = 0;

    buffer_write(entry->name, 11, "           ", 11, 0);

    for (i = 0; i < length; i++)
    {

        unsigned int part = (i > dot);
        unsigned char c = name[i];

        if (i == dot)
            continue;

        if (c >= 'a' && c <= 'z')
        {

            cases[part] |= 1;
            c -= 32;

        }

        else if (c >= 'A' && c <= 'Z')
        {

            cases[part] |= 2;

        }

        if (!isshortchar(c))
            return 0;

        entry->name[(part) ? 8 + i - dot - 1 : i] = c;

    }

    if (cases[0] == 3 || cases[1] == 3)
        return 0;

    entry->lowercase = ((cases[0] == 1) ? FAT_LOWERCASE_NAME : 0) | ((cases[1] == 1) ? FAT_LOWERCASE_EXTENSION : 0);

    return 1;

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

    if (id == ROOT && length <= 2 && buffer_match("..", path, length))
        return ROOT;

    getnode(id, &entry);

    if (gettype(&entry) != RECORD_TYPE_DIRECTORY)
        return 0;

    first = getcluster(&entry);

    while ((id = nextentry(first, &offset, &entry, name)))
    {

        if (matchname(name, path, length))
            return (gettype(&entry) == RECORD_TYPE_DIRECTORY && !entry.clusterhigh && !entry.clusterlow) ? ROOT : id;

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

static unsigned int readdirectory(unsigned int root, unsigned int first, unsigned int offset, unsigned int capacity, void *data)
{

    struct record *records = data;
    struct fat_entry entry;
    char name[NAMESIZE];
    unsigned int i = 0;
    unsigned int id;

    if (root && offset < 2)
    {

        if (!offset)
        {

            record_init(&records[0], ROOT, RECORD_TYPE_DIRECTORY, 0, 1, 1, ".");
            record_init(&records[1], ROOT, RECORD_TYPE_DIRECTORY, 0, 1, 2, "..");

            i = 2;

        }

        offset = 0;

    }

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

static unsigned int getdirsector(unsigned int first, unsigned int offset)
{

    unsigned int sector = findsector(first, offset);

    if (!sector && findcluster(first, offset / clustersize, 1))
        sector = findsector(first, offset);

    return sector;

}

static unsigned int putdirentry(unsigned int first, unsigned int offset, void *entry)
{

    unsigned int sector = getdirsector(first, offset);
    unsigned int slot = (offset % sectorsize) / sizeof (struct fat_entry);

    if (!sector)
        return 0;

    buffer_copy((unsigned char *)readsector(sector) + slot * sizeof (struct fat_entry), entry, sizeof (struct fat_entry));
    writesector(sector);

    return sector * (sectorsize / sizeof (struct fat_entry)) + slot;

}

static unsigned int findfree(unsigned int first, unsigned int count)
{

    unsigned int start = 0;
    unsigned int found = 0;
    unsigned int offset;

    for (offset = 0; found < count; offset += sizeof (struct fat_entry))
    {

        unsigned int sector = getdirsector(first, offset);
        unsigned char *data;

        if (!sector)
            return 0xFFFFFFFF;

        data = (unsigned char *)readsector(sector) + ((offset % sectorsize) / sizeof (struct fat_entry)) * sizeof (struct fat_entry);

        if (data[0] == FAT_ENTRY_END)
            return (found) ? start : offset;

        if (data[0] == FAT_ENTRY_DELETED)
        {

            if (!found)
                start = offset;

            found++;

        }

        else
        {

            found = 0;

        }

    }

    return start;

}

static unsigned int hasshortname(unsigned int first, unsigned char *name)
{

    unsigned int offset;

    for (offset = 0; ; offset += sizeof (struct fat_entry))
    {

        unsigned int sector = findsector(first, offset);
        struct fat_entry *entry;

        if (!sector)
            return 0;

        entry = (struct fat_entry *)((unsigned char *)readsector(sector) + ((offset % sectorsize) / sizeof (struct fat_entry)) * sizeof (struct fat_entry));

        if (entry->name[0] == FAT_ENTRY_END)
            return 0;

        if (entry->name[0] != FAT_ENTRY_DELETED && entry->attributes != FAT_ATTRIBUTE_LONGNAME && buffer_match(entry->name, name, 11))
            return 1;

    }

}

static unsigned int islongchar(unsigned char c)
{

    if (c < 0x20 || c >= 0x80)
        return 0;

    switch (c)
    {

    case '\\':
    case '/':
    case ':':
    case '*':
    case '?':
    case '"':
    case '<':
    case '>':
    case '|':
        return 0;

    }

    return 1;

}

static unsigned int makealias(unsigned int first, struct fat_entry *entry, char *name, unsigned int length)
{

    char base[8];
    unsigned int baselength = 0;
    unsigned int dot = length;
    unsigned int tail;
    unsigned int i;

    for (i = 0; i < length; i++)
    {

        if (name[i] == '.')
            dot = i;

    }

    buffer_write(entry->name, 11, "           ", 11, 0);

    for (i = 0; i < dot && baselength < 8; i++)
    {

        unsigned char c = name[i];

        if (c == ' ' || c == '.')
            continue;

        if (c >= 'a' && c <= 'z')
            c -= 32;

        base[baselength++] = (isshortchar(c)) ? c : '_';

    }

    if (!baselength)
        base[baselength++] = '_';

    for (i = dot + 1; i < length && i < dot + 4; i++)
    {

        unsigned char c = name[i];

        if (c >= 'a' && c <= 'z')
            c -= 32;

        entry->name[8 + i - dot - 1] = (isshortchar(c)) ? c : '_';

    }

    for (tail = 1; tail < 1000000; tail++)
    {

        char number[8];
        unsigned int digits = cstring_write_value(number, 8, tail, 10, 0, 0);
        unsigned int keep = (baselength < 7 - digits) ? baselength : 7 - digits;

        buffer_write(entry->name, 8, "        ", 8, 0);
        buffer_write(entry->name, 8, base, keep, 0);
        buffer_write(entry->name, 8, "~", 1, keep);
        buffer_write(entry->name, 8, number, digits, keep + 1);

        if (!hasshortname(first, entry->name))
            return 1;

    }

    return 0;

}

static unsigned int createlongname(unsigned int first, struct fat_entry *entry, char *name, unsigned int length)
{

    unsigned int count = (length + 12) / 13;
    unsigned char checksum;
    unsigned int offset;
    unsigned int i;

    if (length > 255)
        return 0;

    for (i = 0; i < length; i++)
    {

        if (!islongchar(name[i]))
            return 0;

    }

    if (!makealias(first, entry, name, length))
        return 0;

    offset = findfree(first, count + 1);

    if (offset == 0xFFFFFFFF)
        return 0;

    checksum = getchecksum(entry);

    for (i = 0; i < count; i++)
    {

        unsigned int order = count - i;
        struct fat_longname longname;
        unsigned short chars[13];
        unsigned int k;

        for (k = 0; k < 13; k++)
        {

            unsigned int index = (order - 1) * 13 + k;

            chars[k] = (index < length) ? (unsigned char)name[index] : (index == length) ? 0x0000 : 0xFFFF;

        }

        buffer_clear(&longname, sizeof (struct fat_longname));

        longname.order = order | ((i == 0) ? 0x40 : 0);
        longname.attributes = FAT_ATTRIBUTE_LONGNAME;
        longname.checksum = checksum;

        buffer_copy(longname.name1, chars, 10);
        buffer_copy(longname.name2, chars + 5, 12);
        buffer_copy(longname.name3, chars + 11, 4);

        if (!putdirentry(first, offset + i * sizeof (struct fat_entry), &longname))
            return 0;

    }

    return putdirentry(first, offset + count * sizeof (struct fat_entry), entry);

}

static unsigned int createfile(unsigned int parent, char *name, unsigned int length)
{

    unsigned int id = findentry(parent, name, length);
    struct fat_entry directory;
    struct fat_entry entry;
    unsigned int first;
    unsigned int offset;

    if (id)
        return id;

    getnode(parent, &directory);

    if (gettype(&directory) != RECORD_TYPE_DIRECTORY)
        return 0;

    buffer_clear(&entry, sizeof (struct fat_entry));

    entry.attributes = FAT_ATTRIBUTE_ARCHIVE;
    first = getcluster(&directory);

    setcreated(&entry);

    if (!makeshortname(&entry, name, length))
        return createlongname(first, &entry, name, length);

    offset = findfree(first, 1);

    if (offset == 0xFFFFFFFF)
        return 0;

    return putdirentry(first, offset, &entry);

}

static unsigned int writefile(unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct fat_entry entry;
    unsigned int first;
    unsigned int cluster;
    unsigned int sector;

    if (id == ROOT)
        return 0;

    getnode(id, &entry);

    if (gettype(&entry) != RECORD_TYPE_NORMAL)
        return 0;

    if (count > sectorsize - offset % sectorsize)
        count = sectorsize - offset % sectorsize;

    first = (entry.clusterhigh << 16) | entry.clusterlow;

    if (!first)
    {

        first = allocate();

        if (!first)
            return 0;

        entry.clusterhigh = first >> 16;
        entry.clusterlow = first & 0xFFFF;

        putnode(id, &entry);

    }

    cluster = findcluster(first, offset / clustersize, 1);

    if (!cluster)
        return 0;

    sector = clustersector(cluster) + (offset % clustersize) / sectorsize;

    buffer_write(readsector(sector), sectorsize, buffer, count, offset % sectorsize);
    writesector(sector);

    if (offset + count > entry.size)
    {

        entry.size = offset + count;

        setmodified(&entry);
        putnode(id, &entry);

    }

    else if (setmodified(&entry))
    {

        putnode(id, &entry);

    }

    return count;

}

static unsigned int isempty(unsigned int first)
{

    unsigned int offset = 0;
    struct fat_entry entry;
    char name[NAMESIZE];

    while (nextentry(first, &offset, &entry, name))
    {

        if (!buffer_match(entry.name, ".          ", 11) && !buffer_match(entry.name, "..         ", 11))
            return 0;

    }

    return 1;

}

static unsigned int getentryid(unsigned int first, unsigned int offset)
{

    unsigned int sector = findsector(first, offset);

    return (sector) ? sector * (sectorsize / sizeof (struct fat_entry)) + (offset % sectorsize) / sizeof (struct fat_entry) : 0;

}

static void removelongname(unsigned int parent, unsigned int id)
{

    struct fat_entry entry;
    unsigned int offset = 0;
    unsigned int start = 0;
    unsigned int current;
    unsigned int first;

    getnode(parent, &entry);

    first = getcluster(&entry);

    while ((current = getentryid(first, offset)))
    {

        getnode(current, &entry);

        if (entry.name[0] == FAT_ENTRY_END)
            return;

        if (current == id)
        {

            for (; start < offset; start += sizeof (struct fat_entry))
            {

                current = getentryid(first, start);

                getnode(current, &entry);

                entry.name[0] = FAT_ENTRY_DELETED;

                putnode(current, &entry);

            }

            return;

        }

        offset += sizeof (struct fat_entry);

        if (entry.attributes != FAT_ATTRIBUTE_LONGNAME || entry.name[0] == FAT_ENTRY_DELETED)
            start = offset;

    }

}

static unsigned int removenode(unsigned int parent, unsigned int id)
{

    struct fat_entry entry;
    unsigned int freed = 0;
    unsigned int cluster;

    if (id == ROOT)
        return 0;

    getnode(id, &entry);

    if (gettype(&entry) == RECORD_TYPE_DIRECTORY && !isempty(getcluster(&entry)))
        return 0;

    cluster = (entry.clusterhigh << 16) | entry.clusterlow;

    while (cluster >= 2 && cluster < FAT_CLUSTER_END)
    {

        unsigned int next = nextcluster(cluster);

        setcluster(cluster, 0);

        cluster = next;
        freed++;

    }

    addfree(freed, 0);
    removelongname(parent, id);

    entry.name[0] = FAT_ENTRY_DELETED;

    putnode(id, &entry);

    cachefirst = 0;

    return 1;

}

static void oncreaterequest(struct message *message)
{

    struct event_createrequest *request = message->data;
    struct event_createresponse response;

    response.id = createfile((request->parent) ? request->parent : ROOT, (char *)(request + 1), request->count);

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
        response->count = readdirectory(request->id == ROOT, getcluster(&entry), request->offset, MESSAGE_SIZE - sizeof (struct event_readresponse), response + 1);
    else
        response->count = readfile(getcluster(&entry), entry.size, request->offset, request->count, MESSAGE_SIZE - sizeof (struct event_readresponse), response + 1);

    channel_send(0, message->source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + response->count, data);

}

static void onremoverequest(struct message *message)
{

    struct event_removerequest *request = message->data;
    struct event_removeresponse response;

    response.status = removenode(request->parent, request->id);

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

    struct event_writerequest *request = message->data;
    struct event_writeresponse response;

    response.count = writefile(request->id, request->offset, request->count, request + 1);

    channel_send(0, message->source, EVENT_WRITERESPONSE, sizeof (struct event_writeresponse), &response);

}

static void onmain(struct message *message)
{

    block = channel_lookup(option_getstring("block-service"));
    clock = channel_lookup(option_getstring("clock-service"));
    partoffset = option_getdecimal("partoffset");

    if (block)
    {

        struct fat fat;

        channel_send(1, block, EVENT_INFO, 0, 0);

        if (!channel_wait(1, block, EVENT_BLOCKINFO, sizeof (struct event_blockinfo), &blockinfo))
            return;

        sectorsize = 512;

        buffer_copy(&fat, readsector(0), sizeof (struct fat));

        if (fat_validate(&fat))
        {

            struct fat32 *fat32 = (struct fat32 *)fat.extended_section;
            unsigned int total = (fat.total_sectors_16) ? fat.total_sectors_16 : fat.total_sectors_32;
            unsigned int clusters;

            sectorsize = fat.bytes_per_sector;
            clustersize = sectorsize * fat.sectors_per_cluster;
            fatstart = fat.reserved_sector_count;
            fatcount = fat.table_count;
            fatsize = (fat.table_size_16) ? fat.table_size_16 : fat32->table_size_32;
            rootstart = fatstart + fatcount * fatsize;
            rootsectors = (fat.root_entry_count * sizeof (struct fat_entry) + sectorsize - 1) / sectorsize;
            datastart = rootstart + rootsectors;
            clusters = (total - datastart) / fat.sectors_per_cluster;
            clustercount = clusters + 2;
            fattype = (!fat.table_size_16) ? 32 : (clusters < 4085) ? 12 : 16;
            rootcluster = (fattype == 32) ? fat32->root_cluster : 0;
            fsinfo = (fattype == 32) ? fat32->fat_info : 0;

            call_announce(0, cstring_length(option_getstring("service")), option_getstring("service"));
            channel_hold(0);

        }

    }

}

void init(void)
{

    option_add("service", "fat");
    option_add("block-service", "block");
    option_add("clock-service", "clock");
    option_add("partoffset", "0");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_CREATEREQUEST, oncreaterequest);
    channel_bind(EVENT_READREQUEST, onreadrequest);
    channel_bind(EVENT_REMOVEREQUEST, onremoverequest);
    channel_bind(EVENT_STATREQUEST, onstatrequest);
    channel_bind(EVENT_WALKREQUEST, onwalkrequest);
    channel_bind(EVENT_WRITEREQUEST, onwriterequest);

}

