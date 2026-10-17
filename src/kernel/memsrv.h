#define MEMSRV_FILES                    8
#define MEMSRV_FILESIZE                 0x00800000
#define MEMSRV_NAMESIZE                 32

void memsrv_untask(unsigned int itask);
void memsrv_setup(unsigned long paddress, void (*unmap)(unsigned long vaddress, unsigned int size));
