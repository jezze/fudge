unsigned long page_getbase(void);
unsigned int page_getfree(void);
unsigned int page_alloc(unsigned int itask);
unsigned int page_give(unsigned int itask, unsigned int page, unsigned int target, unsigned int count, unsigned int source);
struct page_handoff page_take(unsigned int itask, unsigned int ichannel);
unsigned int page_free(unsigned int itask, unsigned int page);
void page_untask(unsigned int itask);
void page_setup(unsigned long address);
