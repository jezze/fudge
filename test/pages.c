#include <fudge.h>
#include <abi.h>

#define FREESIZE                        32

struct block
{

    unsigned char bytes[PAGE_SIZE];

};

static unsigned int held[PAGE_TASKLIMIT];

static unsigned int allocall(void)
{

    unsigned int count = 0;

    while (count < PAGE_TASKLIMIT && (held[count] = call_pagealloc(0)))
        count++;

    return count;

}

static void freeall(unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
        call_pagefree(held[i]);

}

static unsigned int readfree(void)
{

    char data[FREESIZE];
    unsigned int count = system_feed(0, 0, data, FREESIZE, "mq -query . -format {free} sysinfo:pages.mq");

    return cstring_read_value(data, buffer_findbyte(data, count, '\n'), 10);

}

static void onmain(struct message *message)
{

    struct page_handoff handoff;
    /* services hold pages of their own, such as the disk services' block pages, so what comes back is measured against the start */
    unsigned int before = readfree();
    unsigned int free;
    unsigned int count;
    unsigned int i;

    count = allocall();

    if (count != PAGE_TASKLIMIT)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "a task got %u pages before its limit\n", &count);

    if (call_pagealloc(0))
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "a task at its limit got a page\n");

    for (i = 0; i < count; i++)
    {

        struct block *block = (struct block *)((unsigned long)PAGE_VBASE + (unsigned long)(held[i] - 1) * PAGE_SIZE);

        block->bytes[0] = i;
        block->bytes[PAGE_SIZE - 1] = i;

    }

    for (i = 0; i < count; i++)
    {

        struct block *block = (struct block *)((unsigned long)PAGE_VBASE + (unsigned long)(held[i] - 1) * PAGE_SIZE);

        if (block->bytes[0] != (unsigned char)i || block->bytes[PAGE_SIZE - 1] != (unsigned char)i)
            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "page %u does not keep its bytes\n", &held[i]);

    }

    if (call_pagetake(0, &handoff, 0) != MESSAGE_RETRY)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "take without a page did not ask to retry\n");

    if (count && call_pagegive(held[0], message->source, PAGE_SIZE + 1) == MESSAGE_OK)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "a count larger than a page was given\n");

    freeall(count);

    if (count && call_pagefree(held[0]) == MESSAGE_OK)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "a page was freed twice\n");

    count = allocall();

    if (count != PAGE_TASKLIMIT)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "a task got %u pages after freeing\n", &count);

    freeall(count);

    free = readfree();

    if (free != before)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "%u pages free at the end, %u at the start\n", &free, &before);
        system_run(message->source, "mq -query .held -format \"held page {page} owner {owner} origin {origin} queued {queued} count {count} source {source}\" sysinfo:pages.mq");

    }

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

