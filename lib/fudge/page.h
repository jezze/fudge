#define PAGE_SIZE                       0x1000
#define PAGE_COUNT                      256
#define PAGE_TASKLIMIT                  8
#define PAGE_VBASE                      0x41000000
#define PAGE_FLAG_BLOCK                 1

struct page_handoff
{

    unsigned int page;
    unsigned int count;
    unsigned int source;

};
