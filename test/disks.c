#include <fudge.h>
#include <abi.h>

/* the disk test/run attaches: the same pattern file on a FAT and an ext2 partition, read through their services */
#define FILES                           2
#define SIZE                            40000
#define WRITTEN                         9000

struct block
{

    unsigned char bytes[PAGE_SIZE];

};

static char *paths[FILES] = {
    "tfat:pattern.txt",
    "text2:pattern.txt"
};

static struct block buffer;

static unsigned int matches(unsigned int target, unsigned int id, unsigned int size)
{

    unsigned int offset = 0;
    unsigned int count;

    while ((count = fs_read(1, target, id, buffer.bytes, PAGE_SIZE, offset)))
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            if (buffer.bytes[i] != 'a' + (offset + i) % 26)
                return 0;

        }

        offset += count;

    }

    return offset == size;

}

/* written across several pages, then read back */
static unsigned int writeback(char *path)
{

    unsigned int target = fs_auth(path);
    unsigned int parent = (target) ? fs_walk(1, target, 0, "tfat:") : 0;
    unsigned int id = (parent) ? fs_create(1, target, parent, "written.txt", 11) : 0;
    unsigned int offset;

    if (!id)
        return 0;

    for (offset = 0; offset < WRITTEN; offset += PAGE_SIZE)
    {

        unsigned int count = (WRITTEN - offset < PAGE_SIZE) ? WRITTEN - offset : PAGE_SIZE;
        unsigned int i;

        for (i = 0; i < count; i++)
            buffer.bytes[i] = 'a' + (offset + i) % 26;

        if (fs_write_all(1, target, id, buffer.bytes, count, offset) != count)
            return 0;

    }

    return matches(target, id, WRITTEN);

}

static unsigned int check(char *path)
{

    unsigned int target = fs_auth(path);
    unsigned int id = (target) ? fs_walk(1, target, 0, path) : 0;

    return id && matches(target, id, SIZE);

}

static void onmain(struct message *message)
{

    unsigned int found = 0;
    unsigned int i;

    for (i = 0; i < FILES; i++)
    {

        if (fs_auth(paths[i]))
            found++;

    }

    if (!found)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "skipped: no disk\n");

        return;

    }

    for (i = 0; i < FILES; i++)
    {

        if (!check(paths[i]))
            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "%s did not read back\n", paths[i]);

    }

    if (!writeback("tfat:written.txt"))
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "tfat:written.txt did not read back\n");

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

