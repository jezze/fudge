void *page_alloc(void);
void *page_tryalloc(void);
void *page_address(unsigned int page);
unsigned int page_number(void *data);
unsigned int page_hand(void *data, unsigned int target);
unsigned int page_free(void *data);
