struct page_data
{

    void *data;
    unsigned int count;

};

void *page_alloc(void);
void *page_tryalloc(void);
unsigned int page_give(void *data, unsigned int target, unsigned int count);
struct page_data page_take(void);
unsigned int page_free(void *data);
