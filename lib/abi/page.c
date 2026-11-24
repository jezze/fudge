#include <fudge.h>
#include "call.h"
#include "page.h"

void *page_address(unsigned int page)
{

    return (page && page <= PAGE_COUNT) ? (void *)((unsigned long)PAGE_VBASE + (unsigned long)(page - 1) * PAGE_SIZE) : 0;

}

unsigned int page_number(void *data)
{

    unsigned long address = (unsigned long)data;

    if (address < PAGE_VBASE || address >= PAGE_VBASE + (unsigned long)PAGE_COUNT * PAGE_SIZE)
        return 0;

    return (address - PAGE_VBASE) / PAGE_SIZE + 1;

}

/* the page goes to the target along with a request naming it; this returns the name */
unsigned int page_hand(void *data, unsigned int target)
{

    unsigned int page = page_number(data);

    return (page && call_pagehand(page, target) == MESSAGE_OK) ? page : 0;

}

void *page_alloc(void)
{

    unsigned int page;

    while (!(page = call_pagealloc(PAGE_FLAG_BLOCK)));

    return page_address(page);

}

void *page_tryalloc(void)
{

    return page_address(call_pagealloc(0));

}

unsigned int page_free(void *data)
{

    return call_pagefree(page_number(data)) == MESSAGE_OK;

}

