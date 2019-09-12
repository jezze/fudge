#include <fudge.h>
#include <abi.h>

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    struct job jobs[32];

    job_interpret(jobs, 32, channel, mdata, msize, 0);

}

void main(void)
{

    struct channel channel;
    struct {struct event_header header; struct event_file file;} message;
    unsigned int id;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);

    if (!file_walk2(FILE_CP, "/bin/slang"))
        return;

    if (!file_walk2(FILE_C0, "/config/base.slang"))
        return;

    if (!file_walk2(FILE_C1, "/config/arch.slang"))
        return;

    if (!file_walk2(FILE_C2, "/config/init.slang"))
        return;

    id = call_spawn();

    if (!id)
        return;

    event_create(&message.header, EVENT_FILE, sizeof (struct event_file));

    message.file.descriptor = FILE_P0;

    call_place(id, &message, message.header.length);

    message.file.descriptor = FILE_P1;

    call_place(id, &message, message.header.length);

    message.file.descriptor = FILE_P2;

    call_place(id, &message, message.header.length);
    event_create(&message.header, EVENT_STOP, 0);
    call_place(id, &message, message.header.length);
    channel_listen(&channel);

}

