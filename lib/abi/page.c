#include <fudge.h>
#include "call.h"
#include "page.h"

static void *getdata(unsigned int page)
{

    return (page) ? (void *)((unsigned long)PAGE_VBASE + (unsigned long)(page - 1) * PAGE_SIZE) : 0;

}

static unsigned int getpage(void *data)
{

    unsigned long address = (unsigned long)data;

    if (address < PAGE_VBASE || address >= PAGE_VBASE + (unsigned long)PAGE_COUNT * PAGE_SIZE)
        return 0;

    return (address - PAGE_VBASE) / PAGE_SIZE + 1;

}

void *page_alloc(void)
{

    unsigned int page;

    while (!(page = call_pagealloc(PAGE_FLAG_BLOCK)));

    return getdata(page);

}

void *page_tryalloc(void)
{

    return getdata(call_pagealloc(0));

}

unsigned int page_give(void *data, unsigned int target, unsigned int count)
{

    return call_pagegive(getpage(data), target, count) == MESSAGE_OK;

}

struct page_data page_take(void)
{

    struct page_handoff handoff;
    struct page_data result;
    unsigned int status;

    while ((status = call_pagetake(&handoff, PAGE_FLAG_BLOCK)) == MESSAGE_RETRY);

    result.data = (status == MESSAGE_OK) ? getdata(handoff.page) : 0;
    result.count = (status == MESSAGE_OK) ? handoff.count : 0;

    return result;

}

unsigned int page_free(void *data)
{

    return call_pagefree(getpage(data)) == MESSAGE_OK;

}

