#include <fudge.h>
#include <abi.h>
#include <compression.h>

#define NUM_NAMES                       8
#define PATHSIZE                        1024
#define HEADERSIZE                      (46 + PATHSIZE)
#define CRCTABLE                        256
#define EOCDSIZE                        22
#define EOCDMAX                         (EOCDSIZE + 0xFFFF)

static unsigned int crctable[CRCTABLE];
static char archive[PATHSIZE];
static char destination[PATHSIZE];
static unsigned int paths;
static unsigned int memtarget;
static unsigned int memparent;
static unsigned int memid;
static unsigned char *data;

static unsigned int read16(unsigned char *data)
{

    return data[0] | (data[1] << 8);

}

static unsigned int read32(unsigned char *data)
{

    return data[0] | (data[1] << 8) | (data[2] << 16) | ((unsigned int)data[3] << 24);

}

static unsigned int crc32(unsigned char *data, unsigned int count)
{

    unsigned int crc = 0xFFFFFFFF;
    unsigned int i;

    for (i = 0; i < count; i++)
        crc = crctable[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);

    return ~crc;

}

/* a file of our own in mem:, named unzip and the first free number */
static unsigned int createmem(void)
{

    char name[32];
    unsigned int i;

    memtarget = fs_auth("mem:");
    memparent = (memtarget) ? fs_walk(1, memtarget, 0, "mem:") : 0;

    for (i = 0; i < NUM_NAMES && memparent && !memid; i++)
    {

        cstring_write_fmt(name, 32, 0, "unzip%u\\0", &i);

        memid = fs_create(1, memtarget, memparent, name, cstring_length(name));

    }

    if (memid)
        data = fs_mmap(1, memtarget, memid);

    return data != 0;

}

/* the mem: file refuses writes past its end, so writing the last byte tells whether count + extra bytes fit */
static unsigned int fit(unsigned int count, unsigned int extra)
{

    unsigned char zero = 0;

    if (count + extra < count)
        return 0;

    return !(count + extra) || fs_write(1, memtarget, memid, &zero, 1, count + extra - 1) == 1;

}

static unsigned int inflate(unsigned char *in, unsigned int icount, unsigned char *out, unsigned int ocount, unsigned int *consumed)
{

    struct deflate d;

    deflate_init(&d, out, ocount, in, icount);

    if (deflate_decompress(&d) != DEFLATE_OK)
        return 0;

    *consumed = d.ioffset;

    return d.ooffset == ocount;

}

static unsigned int isdirectory(unsigned int target, unsigned int id)
{

    struct record record;

    return fs_stat(1, target, id, &record) && record.type == RECORD_TYPE_DIRECTORY;

}

static unsigned int safename(char *name)
{

    unsigned int i;

    if (!name[0] || name[0] == '/' || cstring_match(name, ".."))
        return 0;

    for (i = 0; name[i]; i++)
    {

        if (name[i] == ':' || name[i] == '\\')
            return 0;

        if (name[i] == '.' && name[i + 1] == '.' && (!i || name[i - 1] == '/') && (!name[i + 2] || name[i + 2] == '/'))
            return 0;

    }

    return 1;

}

static unsigned int writefile(unsigned int source, unsigned int target, unsigned int directory, char *name, unsigned char *output, unsigned int count)
{

    char parentpath[PATHSIZE];
    unsigned int length = fs_dirlength(name);
    char *base = name + length;
    unsigned int parent = directory;
    unsigned int id;

    if (length)
    {

        buffer_write(parentpath, PATHSIZE, name, length, 0);

        parentpath[length] = '\0';
        parent = fs_walk(1, target, directory, parentpath);

        if (!parent)
        {

            channel_send_fmt(0, source, EVENT_ERROR, "Directory not found: %s\n", parentpath);

            return 0;

        }

    }

    if (!base[0])
        return 1;

    id = fs_walk(1, target, parent, base);

    if (id)
    {

        if (isdirectory(target, id) || !fs_remove(1, target, parent, id))
        {

            channel_send_fmt(0, source, EVENT_ERROR, "File could not be replaced: %s\n", name);

            return 0;

        }

    }

    id = fs_create(1, target, parent, base, cstring_length(base));

    if (!id)
    {

        channel_send_fmt(0, source, EVENT_ERROR, "File could not be created: %s\n", name);

        return 0;

    }

    if (fs_write_all(1, target, id, output, count, 0) != count)
    {

        channel_send_fmt(0, source, EVENT_ERROR, "File could not be written: %s\n", name);

        return 0;

    }

    channel_send_fmt(0, source, EVENT_DATA, "%s\n", name);

    return 1;

}

static void gunzip(unsigned int source, unsigned int starget, unsigned int sid, unsigned int size, unsigned int dtarget, unsigned int did)
{

    char name[PATHSIZE];
    char *base = archive + fs_dirlength(archive);
    unsigned int length = cstring_length(base);
    unsigned int offset = 10;
    unsigned int flags;
    unsigned int consumed;
    unsigned int usize;

    if (!fit(size, 0))
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Archive too big: %s\n", archive);

        return;

    }

    if (length > 3 && cstring_match(base + length - 3, ".gz"))
    {

        buffer_write(name, PATHSIZE, base, length - 3, 0);

        name[length - 3] = '\0';

    }

    else if (length > 4 && cstring_match(base + length - 4, ".tgz"))
    {

        buffer_write(name, PATHSIZE, base, length - 4, 0);
        buffer_write(name, PATHSIZE, ".tar", 5, length - 4);

    }

    else
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Name does not end with .gz: %s\n", archive);

        return;

    }

    if (fs_read_full(1, starget, sid, data, size, 0) != size || data[2] != 8 || size < 18)
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Archive could not be read: %s\n", archive);

        return;

    }

    flags = data[3];

    if (flags & 0x04)
        offset += 2 + read16(data + offset);

    if (flags & 0x08)
        for (; offset < size && data[offset]; offset++);

    if (flags & 0x08)
        offset++;

    if (flags & 0x10)
        for (; offset < size && data[offset]; offset++);

    if (flags & 0x10)
        offset++;

    if (flags & 0x02)
        offset += 2;

    usize = read32(data + size - 4);

    if (offset + 8 > size)
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Archive is damaged: %s\n", archive);

        return;

    }

    if (!fit(size, usize))
    {

        channel_send_fmt(0, source, EVENT_ERROR, "File too big: %s\n", archive);

        return;

    }

    if (!inflate(data + offset, size - offset - 8, data + size, usize, &consumed) || crc32(data + size, usize) != read32(data + offset + consumed))
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Archive is damaged: %s\n", archive);

        return;

    }

    writefile(source, dtarget, did, name, data + size, usize);

}

static void extract(unsigned int source, unsigned int starget, unsigned int sid, unsigned char *header, unsigned int dtarget, unsigned int did)
{

    unsigned char local[30];
    char name[PATHSIZE];
    unsigned int method = read16(header + 10);
    unsigned int crc = read32(header + 16);
    unsigned int csize = read32(header + 20);
    unsigned int usize = read32(header + 24);
    unsigned int length = read16(header + 28);
    unsigned int offset = read32(header + 42);
    unsigned char *output;
    unsigned int consumed;

    buffer_write(name, PATHSIZE, header + 46, length, 0);

    name[length] = '\0';

    if (!safename(name))
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Unsafe name skipped: %s\n", name);

        return;

    }

    if (read16(header + 8) & 0x01 || (method != 0 && method != 8))
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Unsupported method or encryption: %s\n", name);

        return;

    }

    if (!fit(csize, (method) ? usize : 0))
    {

        channel_send_fmt(0, source, EVENT_ERROR, "File too big: %s\n", name);

        return;

    }

    if (fs_read_full(1, starget, sid, local, 30, offset) != 30 || read32(local) != 0x04034B50)
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Archive is damaged: %s\n", name);

        return;

    }

    offset += 30 + read16(local + 26) + read16(local + 28);

    output = (method) ? data + csize : data;

    if (fs_read_full(1, starget, sid, data, csize, offset) != csize)
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Archive is damaged: %s\n", name);

        return;

    }

    if ((method && !inflate(data, csize, output, usize, &consumed)) || (!method && csize != usize) || crc32(output, usize) != crc)
    {

        channel_send_fmt(0, source, EVENT_ERROR, "File is damaged: %s\n", name);

        return;

    }

    writefile(source, dtarget, did, name, output, usize);

}

static void unzip(unsigned int source, unsigned int starget, unsigned int sid, unsigned int size, unsigned int dtarget, unsigned int did)
{

    unsigned char header[HEADERSIZE];
    unsigned int tail = (size < EOCDMAX) ? size : EOCDMAX;
    unsigned int entries;
    unsigned int offset;
    unsigned int i;

    if (tail < EOCDSIZE || !fit(tail, 0) || fs_read_full(1, starget, sid, data, tail, size - tail) != tail)
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Archive could not be read: %s\n", archive);

        return;

    }

    for (i = tail - EOCDSIZE + 1; i > 0 && read32(data + i - 1) != 0x06054B50; i--);

    if (!i)
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Archive is damaged: %s\n", archive);

        return;

    }

    entries = read16(data + i - 1 + 10);
    offset = read32(data + i - 1 + 16);

    if (entries == 0xFFFF || offset == 0xFFFFFFFF)
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Zip64 is not supported: %s\n", archive);

        return;

    }

    for (i = 0; i < entries; i++)
    {

        unsigned int count = fs_read_full(1, starget, sid, header, HEADERSIZE, offset);

        if (count < 46 || read32(header) != 0x02014B50 || 46 + read16(header + 28) > count || read16(header + 28) >= PATHSIZE)
        {

            channel_send_fmt(0, source, EVENT_ERROR, "Archive is damaged: %s\n", archive);

            return;

        }

        extract(source, starget, sid, header, dtarget, did);

        offset += 46 + read16(header + 28) + read16(header + 30) + read16(header + 32);

    }

}

static void onpath(struct message *message)
{

    if (!paths)
        buffer_write(archive, PATHSIZE, message->data, cstring_length_zero(message->data), 0);
    else if (paths == 1)
        buffer_write(destination, PATHSIZE, message->data, cstring_length_zero(message->data), 0);

    paths++;

}

static void onterm(struct message *message)
{

    unsigned int starget = fs_auth(archive);
    unsigned int sid = (starget) ? fs_walk(1, starget, 0, archive) : 0;
    unsigned int dtarget = (paths == 2) ? fs_auth(destination) : starget;
    unsigned int did = 0;
    struct record record;
    unsigned char magic[4];

    if (paths < 1 || paths > 2)
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "Usage: unzip <archive> [<directory>]\n");

        return;

    }

    if (!sid || !fs_stat(1, starget, sid, &record) || record.type == RECORD_TYPE_DIRECTORY)
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "File not found: %s\n", archive);

        return;

    }

    if (dtarget)
        did = (paths == 2) ? fs_walk(1, dtarget, 0, destination) : fs_walkparent(1, dtarget, archive);

    if (!did || !isdirectory(dtarget, did))
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "Directory not found: %s\n", (paths == 2) ? destination : archive);

        return;

    }

    if (fs_read_full(1, starget, sid, magic, 4, 0) != 4)
        magic[0] = 0;

    if (!(magic[0] == 0x1F && magic[1] == 0x8B) && !(magic[0] == 'P' && magic[1] == 'K'))
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "Not a zip or gzip file: %s\n", archive);

        return;

    }

    if (createmem())
    {

        if (magic[0] == 0x1F)
            gunzip(message->source, starget, sid, record.size, dtarget, did);
        else
            unzip(message->source, starget, sid, record.size, dtarget, did);

        fs_munmap(1, memtarget, memid);

    }

    else
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "No mem: file could be mapped\n");

    }

    if (memid)
        fs_remove(1, memtarget, memparent, memid);

}

void init(void)
{

    unsigned int i;

    for (i = 0; i < CRCTABLE; i++)
    {

        unsigned int crc = i;
        unsigned int j;

        for (j = 0; j < 8; j++)
            crc = (crc & 1) ? 0xEDB88320 ^ (crc >> 1) : crc >> 1;

        crctable[i] = crc;

    }

    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}

