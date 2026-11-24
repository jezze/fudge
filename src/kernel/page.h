struct page_info
{

    unsigned int owner;
    unsigned int origin;
    unsigned int queued;
    unsigned int count;
    unsigned int source;

};

struct page_lease
{

    unsigned int page;
    unsigned int status;
    void *data;

};

unsigned long page_getbase(void);
unsigned int page_getfree(void);
unsigned int page_getheld(unsigned int itask);
struct page_info page_getinfo(unsigned int page);
unsigned int page_alloc(unsigned int itask);
unsigned int page_send(unsigned int itask, unsigned int page, struct mailbox *mailbox, unsigned int event, unsigned int count, unsigned int source);
void page_picked(void *data);
unsigned int page_free(unsigned int itask, unsigned int page);
unsigned int page_hand(unsigned int itask, unsigned int page, unsigned int target);
struct page_lease page_borrow(unsigned int source, unsigned int page);
unsigned int page_answer(unsigned int target, struct page_lease *lease, unsigned int count, unsigned int source);
void page_untask(unsigned int itask);
void page_setup(unsigned long address);
