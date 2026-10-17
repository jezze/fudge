#define MEMSRV_FILES                    16
#define MEMSRV_FILESIZE                 0x02000000
#define MEMSRV_PAGES                    0x4000
#define MEMSRV_PAGESIZE                 0x1000
#define MEMSRV_NAMESIZE                 32

unsigned long memsrv_getpage(unsigned int id, unsigned int offset);
void memsrv_untask(unsigned int itask);
void memsrv_setup(unsigned long paddress, void (*unmap)(unsigned long vaddress, unsigned int size));
