#include <fudge.h>
#include <abi.h>

/* through sh, with tr unchanged in the middle and a slow end, so the stages before it wait on their pages */
#define BYTES                           (PAGE_SIZE * PAGE_TASKLIMIT * 8 + 123)
#define DELAY                           20000
#define OUTPUTSIZE                      128

static void onmain(struct message *message)
{

    char output[OUTPUTSIZE];
    char wanted[OUTPUTSIZE];
    unsigned int bytes = BYTES;
    unsigned int delay = DELAY;
    unsigned int count = system_feed(0, 0, output, OUTPUTSIZE, "initrd:test/emit -bytes %u | tr -f a -t b | initrd:test/verify -bytes %u -delay %u", &bytes, &bytes, &delay);
    unsigned int length = cstring_write_fmt(wanted, OUTPUTSIZE, 0, "verify ok %u\n", &bytes);

    if (count != length || !buffer_match(output, wanted, length))
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "pipe gave: %w\n", output, &count);

    /* a background job writes to the log service in the kernel, which reads each page in place */
    if (!system_run(message->source, "initrd:test/emit -bytes 100 &"))
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "background pipe into the log did not run\n");

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

