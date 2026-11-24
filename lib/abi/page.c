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

void *page_address(unsigned int page)
{

    return (page <= PAGE_COUNT) ? getdata(page) : 0;

}

unsigned int page_number(void *data)
{

    return getpage(data);

}

/* the page goes to the target along with a request naming it; this returns the name */
unsigned int page_hand(void *data, unsigned int target)
{

    unsigned int page = getpage(data);

    return (page && call_pagehand(page, target) == MESSAGE_OK) ? page : 0;

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

/* a service in the kernel that cannot take the page now leaves it with this task, as a full mailbox does */
unsigned int page_give(void *data, unsigned int target, unsigned int count)
{

    unsigned int status;

    while ((status = call_pagegive(getpage(data), target, count)) == MESSAGE_RETRY);

    return status == MESSAGE_OK;

}

unsigned int page_free(void *data)
{

    return call_pagefree(getpage(data)) == MESSAGE_OK;

}

