#define FILE_PP                         0x00
#define FILE_PR                         0x01
#define FILE_PW                         0x02
#define FILE_PO                         0x03
#define FILE_PI                         0x04
#define FILE_CP                         0x08
#define FILE_CR                         0x09
#define FILE_CW                         0x0A
#define FILE_CO                         0x0B
#define FILE_CI                         0x0C
#define FILE_L0                         0x10
#define FILE_L1                         0x11
#define FILE_L2                         0x12
#define FILE_L3                         0x13
#define FILE_L4                         0x14
#define FILE_L5                         0x15
#define FILE_L6                         0x16
#define FILE_L7                         0x17
#define FILE_L8                         0x18
#define FILE_L9                         0x19
#define FILE_LA                         0x1A
#define FILE_LB                         0x1B
#define FILE_LC                         0x1C
#define FILE_LD                         0x1D
#define FILE_LE                         0x1E
#define FILE_LF                         0x1F

unsigned int file_walk(unsigned int descriptor, char *path);
unsigned int file_walkfrom(unsigned int descriptor, unsigned int pdescriptor, char *path);
unsigned int file_duplicate(unsigned int descriptor, unsigned int pdescriptor);
unsigned int file_open(unsigned int descriptor);
unsigned int file_close(unsigned int descriptor);
unsigned int file_step(unsigned int descriptor);
unsigned int file_read(unsigned int descriptor, void *buffer, unsigned int count);
unsigned int file_readall(unsigned int descriptor, void *buffer, unsigned int count);
unsigned int file_write(unsigned int descriptor, void *buffer, unsigned int count);
unsigned int file_writeall(unsigned int descriptor, void *buffer, unsigned int count);
unsigned int file_seekread(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset);
unsigned int file_seekreadall(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset);
unsigned int file_seekwrite(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset);
unsigned int file_seekwriteall(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset);
