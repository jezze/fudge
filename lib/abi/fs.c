#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "fs.h"

#define SPAWNNAMES                      64
#define SPAWNCHUNK                      (MESSAGE_SIZE - sizeof (struct message_writerequest))
#define SPAWNNAMESIZE                   32
#define FS_NAMESIZE                     32
#define READSIZE                        0x8000

struct readreply
{

    unsigned int count;
    void *data;

};

/* the service's inode: its name, the part before the colon, walked in the kernel's root */
unsigned int fs_auth(char *path)
{

    unsigned int length = cstring_length(path);
    unsigned int offset = buffer_eachbyte(path, length, ':', 0);

    if (offset > 0)
        length = offset - 1;

    if (length < FS_NAMESIZE)
    {

        char name[FS_NAMESIZE];

        buffer_write(name, FS_NAMESIZE, path, length, 0);

        name[length] = '\0';

        return channel_lookup(name);

    }

    return 0;

}

unsigned int fs_create(unsigned int ichannel, unsigned int target, unsigned int parent, void *buffer, unsigned int count)
{

    char data[MESSAGE_SIZE];
    struct message_createrequest *request = (struct message_createrequest *)data;

    request->parent = parent;
    request->count = buffer_write(data, MESSAGE_SIZE, buffer, count, sizeof (struct message_createrequest));

    if (channel_send(ichannel, target, MESSAGE_TYPE_CREATEREQUEST, sizeof (struct message_createrequest) + request->count, data))
    {

        struct message_createresponse response;

        return (channel_wait(ichannel, target, MESSAGE_TYPE_CREATERESPONSE, sizeof (struct message_createresponse), &response)) ? response.id : 0;

    }

    return 0;

}

unsigned int fs_canonical(char *out, unsigned int size, char *path)
{

    unsigned int length = cstring_length(path);
    unsigned int offset = buffer_eachbyte(path, length, ':', 0);
    unsigned int count = buffer_write(out, size - 1, path, offset, 0);
    unsigned int root = count;
    unsigned int directory = 0;

    while (offset < length)
    {

        char *cp = path + offset;
        unsigned int cl = buffer_findbyte(cp, length - offset, '/');

        directory = (cl < length - offset) || (cl == 1 && cp[0] == '.') || (cl == 2 && cp[0] == '.' && cp[1] == '.');

        if (cl == 2 && cp[0] == '.' && cp[1] == '.')
        {

            if (count > root)
                count--;

            while (count > root && out[count - 1] != '/')
                count--;

        }

        else if (cl && (cl != 1 || cp[0] != '.'))
        {

            count += buffer_write(out, size - 1, cp, cl, count);
            count += buffer_write(out, size - 1, "/", 1, count);

        }

        offset += cl + 1;

    }

    if (!directory && count > root && out[count - 1] == '/')
        count--;

    out[count] = '\0';

    return count;

}

unsigned int fs_absolute(char *out, unsigned int size, char *pwd, char *path)
{

    char full[1024];
    unsigned int service = buffer_eachbyte(pwd, cstring_length(pwd), ':', 0);

    if (buffer_eachbyte(path, cstring_length(path), ':', 0))
        cstring_write_fmt(full, 1024, 0, "%s\\0", path);
    else if (path[0] == '/')
        cstring_write_fmt(full, 1024, 0, "%w%s\\0", pwd, &service, path);
    else
        cstring_write_fmt(full, 1024, 0, "%s%s\\0", pwd, path);

    return fs_canonical(out, size, full);

}

unsigned int fs_dirlength(char *path)
{

    unsigned int length = 0;
    unsigned int i;

    for (i = 0; path[i]; i++)
    {

        if (path[i] == '/' || path[i] == ':')
            length = i + 1;

    }

    return length;

}

unsigned int fs_map(unsigned int ichannel, unsigned int target, unsigned int id)
{

    struct message_maprequest request;

    request.id = id;

    if (channel_send(ichannel, target, MESSAGE_TYPE_MAPREQUEST, sizeof (struct message_maprequest), &request))
    {

        struct message_mapresponse response;

        return (channel_wait(ichannel, target, MESSAGE_TYPE_MAPRESPONSE, sizeof (struct message_mapresponse), &response)) ? response.address : 0;

    }

    return 0;

}

/* maps the file into this task and returns where; unlike fs_map, which tells where the service keeps the file in kernel memory */
void *fs_mmap(unsigned int ichannel, unsigned int target, unsigned int id, unsigned int size)
{

    struct message_mmaprequest request;

    request.id = id;
    request.size = size;

    if (channel_send(ichannel, target, MESSAGE_TYPE_MMAPREQUEST, sizeof (struct message_mmaprequest), &request))
    {

        struct message_mmapresponse response;

        return (channel_wait(ichannel, target, MESSAGE_TYPE_MMAPRESPONSE, sizeof (struct message_mmapresponse), &response)) ? (void *)(unsigned long)response.address : 0;

    }

    return 0;

}

unsigned int fs_munmap(unsigned int ichannel, unsigned int target, unsigned int id)
{

    struct message_munmaprequest request;

    request.id = id;

    if (channel_send(ichannel, target, MESSAGE_TYPE_MUNMAPREQUEST, sizeof (struct message_munmaprequest), &request))
    {

        struct message_munmapresponse response;

        return (channel_wait(ichannel, target, MESSAGE_TYPE_MUNMAPRESPONSE, sizeof (struct message_munmapresponse), &response)) ? response.status : 0;

    }

    return 0;

}

/* the data stays in the reply's slot or in our block buffer until the next pick on ichannel or the next read */
static struct readreply readreply(unsigned int ichannel, unsigned int target, unsigned int id, unsigned int count, unsigned int offset)
{

    struct readreply reply;
    struct message_readrequest request;
    struct message message;

    reply.count = 0;
    reply.data = 0;
    request.id = id;
    request.offset = offset;
    request.count = count;
    request.flags = READ_FLAG_BUFFER;

    if (channel_send(ichannel, target, MESSAGE_TYPE_READREQUEST, sizeof (struct message_readrequest), &request) && channel_poll(ichannel, target, MESSAGE_TYPE_READRESPONSE, &message))
    {

        struct message_readresponse *response = message_validate(MESSAGE_TYPE_READRESPONSE, message.data, message.length);

        if (response)
        {

            reply.count = response->count;
            reply.data = (response->buffer) ? (void *)response->buffer : response + 1;

        }

        if (reply.count > count)
            reply.count = count;

    }

    return reply;

}

unsigned int fs_read(unsigned int ichannel, unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    struct readreply reply = readreply(ichannel, target, id, count, offset);

    buffer_copy(buffer, reply.data, reply.count);

    return reply.count;

}

unsigned int fs_read_full(unsigned int ichannel, unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char *rbuffer = buffer;
    unsigned int rtotal = 0;
    unsigned int roffset;
    unsigned int rcount;

    for (roffset = offset; rtotal < count && (rcount = fs_read(ichannel, target, id, rbuffer + rtotal, count - rtotal, roffset)); roffset += rcount)
        rtotal += rcount;

    return rtotal;

}

unsigned int fs_read_each(unsigned int ichannel, unsigned int source, char *path, void (*each)(unsigned int source, void *buffer, unsigned int count))
{

    unsigned int target = fs_auth(path);
    unsigned int id = (target) ? fs_walk(ichannel, target, 0, path) : 0;
    struct readreply reply;
    unsigned int offset = 0;

    if (!target)
    {

        channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Service not found: %s\n", path);

        return 0;

    }

    if (!id)
    {

        channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Path not found: %s\n", path);

        return 0;

    }

    while ((reply = readreply(ichannel, target, id, READSIZE, offset)).count)
    {

        unsigned char *data = reply.data;
        unsigned int i;

        for (i = 0; i < reply.count; i += MESSAGE_SIZE)
            each(source, data + i, (reply.count - i < MESSAGE_SIZE) ? reply.count - i : MESSAGE_SIZE);

        offset += reply.count;

    }

    return 1;

}

unsigned int fs_read_path(unsigned int ichannel, char *path, void *buffer, unsigned int count)
{

    unsigned int target = fs_auth(path);
    unsigned int id = (target) ? fs_walk(ichannel, target, 0, path) : 0;

    return (id) ? fs_read_full(ichannel, target, id, buffer, count, 0) : 0;

}

unsigned int fs_read_all(unsigned int ichannel, unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    return fs_read_full(ichannel, target, id, buffer, count, offset);

}

unsigned int fs_remove(unsigned int ichannel, unsigned int target, unsigned int parent, unsigned int id)
{

    struct message_removerequest request;

    request.parent = parent;
    request.id = id;

    if (channel_send(ichannel, target, MESSAGE_TYPE_REMOVEREQUEST, sizeof (struct message_removerequest), &request))
    {

        struct message_removeresponse response;

        return (channel_wait(ichannel, target, MESSAGE_TYPE_REMOVERESPONSE, sizeof (struct message_removeresponse), &response)) ? response.status : 0;

    }

    return 0;

}

unsigned int fs_stat(unsigned int ichannel, unsigned int target, unsigned int id, struct record *record)
{

    struct message_statrequest request;

    request.id = id;

    if (channel_send(ichannel, target, MESSAGE_TYPE_STATREQUEST, sizeof (struct message_statrequest), &request))
    {

        unsigned char data[MESSAGE_SIZE];
        struct message_statresponse *response;

        response = message_validate(MESSAGE_TYPE_STATRESPONSE, data, channel_wait(ichannel, target, MESSAGE_TYPE_STATRESPONSE, MESSAGE_SIZE, data));

        if (!response || response->count > sizeof (struct record))
            return 0;

        buffer_copy(record, response + 1, response->count);

        return response->count;

    }

    return 0;

}

unsigned int fs_walk(unsigned int ichannel, unsigned int target, unsigned int parent, char *path)
{

    char data[MESSAGE_SIZE];
    struct message_walkrequest *request = (struct message_walkrequest *)data;

    /* TODO: Handle this stuff better */
    path += buffer_eachbyte(path, cstring_length(path), ':', 0);

    request->parent = parent;
    request->length = buffer_write(data, MESSAGE_SIZE, path, cstring_length(path), sizeof (struct message_walkrequest));

    if (channel_send(ichannel, target, MESSAGE_TYPE_WALKREQUEST, sizeof (struct message_walkrequest) + request->length, data))
    {

        struct message_walkresponse response;

        return (channel_wait(ichannel, target, MESSAGE_TYPE_WALKRESPONSE, sizeof (struct message_walkresponse), &response)) ? response.id : 0;

    }

    return 0;

}

unsigned int fs_walkparent(unsigned int ichannel, unsigned int target, char *path)
{

    unsigned int length = fs_dirlength(path);

    if (length < 1024)
    {

        char directory[1024];

        buffer_write(directory, 1024, path, length, 0);

        directory[length] = '\0';

        return fs_walk(ichannel, target, 0, directory);

    }

    return 0;

}

unsigned int fs_write(unsigned int ichannel, unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    char data[MESSAGE_SIZE];
    struct message_writerequest *request = (struct message_writerequest *)data;

    request->id = id;
    request->offset = offset;
    request->count = buffer_write(data, MESSAGE_SIZE, buffer, count, sizeof (struct message_writerequest));

    if (channel_send(ichannel, target, MESSAGE_TYPE_WRITEREQUEST, sizeof (struct message_writerequest) + request->count, data))
    {

        struct message_writeresponse response;

        return (channel_wait(ichannel, target, MESSAGE_TYPE_WRITERESPONSE, sizeof (struct message_writeresponse), &response)) ? response.count : 0;

    }

    return 0;

}

unsigned int fs_write_all(unsigned int ichannel, unsigned int target, unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char *b = buffer;
    unsigned int c = 0;

    while (c < count)
    {

        unsigned int written = fs_write(ichannel, target, id, b + c, count - c, offset + c);

        if (!written)
            break;

        c += written;

    }

    return c;

}

static unsigned int copyfile(unsigned int ichannel, unsigned int target, unsigned int id, unsigned int memtarget, unsigned int memid)
{

    char buffer[SPAWNCHUNK];
    unsigned int offset = 0;
    unsigned int count;

    while ((count = fs_read(ichannel, target, id, buffer, SPAWNCHUNK, offset)))
    {

        if (fs_write_all(ichannel, memtarget, memid, buffer, count, offset) != count)
            return 0;

        offset += count;

    }

    return offset;

}

/* programs run from mem: only: copy the file into a mem: file of our own, named spawn and the first free number, spawn it and remove it */
static unsigned int spawn(unsigned int ichannel, unsigned int target, unsigned int id)
{

    unsigned int memtarget = fs_auth("mem:");
    unsigned int memparent = (memtarget) ? fs_walk(ichannel, memtarget, 0, "mem:") : 0;
    unsigned int memid = 0;
    unsigned int inode = 0;
    unsigned int i;

    for (i = 0; i < SPAWNNAMES && memparent && !memid; i++)
    {

        char name[SPAWNNAMESIZE];

        cstring_write_fmt(name, SPAWNNAMESIZE, 0, "spawn%u\\0", &i);

        memid = fs_create(ichannel, memtarget, memparent, name, cstring_length(name));

    }

    if (memid)
    {

        if (copyfile(ichannel, target, id, memtarget, memid))
            inode = call_spawn(memtarget, memid);

        fs_remove(ichannel, memtarget, memparent, memid);

    }

    return inode;

}

unsigned int fs_spawn(unsigned int ichannel, char *path)
{

    unsigned int target = fs_auth(path);

    if (target)
    {

        unsigned int id = fs_walk(ichannel, target, 0, path);

        if (id)
            return spawn(ichannel, target, id);

    }

    return 0;

}

unsigned int fs_spawn_relative(unsigned int ichannel, char *path, char *parent)
{

    unsigned int target = fs_auth(parent);

    if (target)
    {

        unsigned int id = fs_walk(ichannel, target, fs_walk(ichannel, target, 0, parent), path);

        if (id)
            return spawn(ichannel, target, id);

    }

    return 0;

}

