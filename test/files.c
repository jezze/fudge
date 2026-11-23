#include <fudge.h>
#include <abi.h>

/* larger than the pages a task may hold, read twice: whole pages, and odd pieces at shifting offsets */
#define PATH                            "initrd:bin/wm"
#define PIECE                           1000

static unsigned char whole[PAGE_SIZE];
static unsigned char pieces[PAGE_SIZE];

static void onmain(struct message *message)
{

    unsigned int target = fs_auth(PATH);
    unsigned int id = (target) ? fs_walk(1, target, 0, PATH) : 0;
    unsigned int offset = 0;
    unsigned int wrong = 0;
    struct record record;

    if (!id || !fs_stat(1, target, id, &record) || record.size <= PAGE_SIZE * PAGE_TASKLIMIT)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "%s is missing or not larger than %u pages\n", PATH, &record.size);

        return;

    }

    while (offset < record.size)
    {

        unsigned int count = fs_read_full(1, target, id, whole, PAGE_SIZE, offset);
        unsigned int done = 0;

        if (!count)
            break;

        while (done < count)
        {

            unsigned int size = (count - done < PIECE) ? count - done : PIECE;

            if (fs_read_full(1, target, id, pieces + done, size, offset + done) != size)
                break;

            done += size;

        }

        if (done != count || !buffer_match(whole, pieces, count))
            wrong++;

        offset += count;

    }

    if (offset != record.size || wrong)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "read %u of %u bytes, %u pages differ\n", &offset, &record.size, &wrong);

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

