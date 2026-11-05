#include <fudge.h>
#include <abi.h>
#include <disk.h>

#define PATHSIZE                        1024
#define COPYSIZE                        0x1000

static char archive[PATHSIZE];
static char destination[PATHSIZE];
static unsigned int paths;

static unsigned int fieldlength(char *field, unsigned int size)
{

    unsigned int length;

    for (length = 0; length < size && field[length]; length++);

    return length;

}

static unsigned int isdirectory(unsigned int target, unsigned int id)
{

    struct record record;

    return fs_stat(1, target, id, &record) && record.type == RECORD_TYPE_DIRECTORY;

}

static unsigned int safename(char *name)
{

    if (name[0] && name[0] != '/' && !cstring_match(name, ".."))
    {

        unsigned int i;

        for (i = 0; name[i]; i++)
        {

            if (name[i] == ':' || name[i] == '\\')
                return 0;

            if (name[i] == '.' && name[i + 1] == '.' && (!i || name[i - 1] == '/') && (!name[i + 2] || name[i + 2] == '/'))
                return 0;

        }

        return 1;

    }

    return 0;

}

/* ustar keeps long names as prefix/name; both fields may fill up without a NUL */
static void readname(struct tar_header *header, char *name)
{

    unsigned int plength = (buffer_match(header->magic, "ustar", 5)) ? fieldlength(header->prefix, sizeof (header->prefix)) : 0;
    unsigned int nlength = fieldlength(header->name, sizeof (header->name));
    unsigned int offset = 0;

    if (plength)
    {

        offset += buffer_write(name, PATHSIZE, header->prefix, plength, offset);
        offset += buffer_write(name, PATHSIZE, "/", 1, offset);

    }

    offset += buffer_write(name, PATHSIZE, header->name, nlength, offset);

    name[offset] = '\0';

}

static unsigned int copyfile(unsigned int starget, unsigned int sid, unsigned int offset, unsigned int size, unsigned int target, unsigned int id)
{

    char buffer[COPYSIZE];
    unsigned int total = 0;

    while (total < size)
    {

        unsigned int count = fs_read_full(1, starget, sid, buffer, (size - total < COPYSIZE) ? size - total : COPYSIZE, offset + total);

        if (!count || fs_write_all(1, target, id, buffer, count, total) != count)
            return 0;

        total += count;

    }

    return 1;

}

static void writefile(unsigned int source, unsigned int starget, unsigned int sid, unsigned int offset, unsigned int size, unsigned int target, unsigned int directory, char *name)
{

    unsigned int length = fs_dirlength(name);
    char *base = name + length;
    unsigned int parent = directory;
    unsigned int id;

    if (!safename(name) || !base[0])
    {

        channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Unsafe name skipped: %s\n", name);

        return;

    }

    if (length)
    {

        char parentpath[PATHSIZE];

        buffer_write(parentpath, PATHSIZE, name, length, 0);

        parentpath[length] = '\0';
        parent = fs_walk(1, target, directory, parentpath);

        if (!parent)
        {

            channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Directory not found: %s\n", parentpath);

            return;

        }

    }

    id = fs_walk(1, target, parent, base);

    if (id)
    {

        if (isdirectory(target, id) || !fs_remove(1, target, parent, id))
        {

            channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "File could not be replaced: %s\n", name);

            return;

        }

    }

    id = fs_create(1, target, parent, base, cstring_length(base));

    if (!id)
    {

        channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "File could not be created: %s\n", name);

        return;

    }

    if (!copyfile(starget, sid, offset, size, target, id))
    {

        channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "File could not be written: %s\n", name);

        return;

    }

    channel_send_fmt(0, source, MESSAGE_TYPE_DATA, "%s\n", name);

}

static void untar(unsigned int source, unsigned int starget, unsigned int sid, unsigned int size, unsigned int dtarget, unsigned int did)
{

    struct tar_header header;
    unsigned int skipnext = 0;
    unsigned int offset = 0;

    while (offset < size)
    {

        char name[PATHSIZE];
        unsigned int fsize;

        if (size - offset < TAR_BLOCK_SIZE || fs_read_full(1, starget, sid, &header, TAR_BLOCK_SIZE, offset) != TAR_BLOCK_SIZE)
        {

            channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Archive is damaged: %s\n", archive);

            return;

        }

        if (!header.name[0])
            return;

        fsize = tar_readvalue(header.size, sizeof (header.size));

        if (!tar_validate(&header) || fsize > size - offset - TAR_BLOCK_SIZE)
        {

            channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Archive is damaged: %s\n", archive);

            return;

        }

        readname(&header, name);

        if (skipnext)
        {

            channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Long name not supported, skipped: %s\n", name);

            skipnext = 0;

        }

        else
        {

            switch (header.typeflag[0])
            {

            case 'L':
            case 'K':
                skipnext = 1;

                break;

            case 'x':
            case 'g':
            case TAR_TYPEFLAG_DIRECTORY:
                break;

            case TAR_TYPEFLAG_REGULAR:
            case '\0':
                writefile(source, starget, sid, offset + TAR_BLOCK_SIZE, fsize, dtarget, did, name);

                break;

            default:
                channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Unsupported entry skipped: %s\n", name);

                break;

            }

        }

        offset = tar_next(&header, offset);

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

    if (paths < 1 || paths > 2)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Usage: untar <archive> [<directory>]\n");

        return;

    }

    if (!sid || !fs_stat(1, starget, sid, &record) || record.type == RECORD_TYPE_DIRECTORY)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "File not found: %s\n", archive);

        return;

    }

    if (dtarget)
        did = (paths == 2) ? fs_walk(1, dtarget, 0, destination) : fs_walkparent(1, dtarget, archive);

    if (!did || !isdirectory(dtarget, did))
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Directory not found: %s\n", (paths == 2) ? destination : archive);

        return;

    }

    untar(message->source, starget, sid, record.size, dtarget, did);

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_PATH, onpath);
    channel_bind(MESSAGE_TYPE_TERM, onterm);

}

