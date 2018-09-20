#define FILE_PP                         0x00
#define FILE_PR                         0x01
#define FILE_PW                         0x02
#define FILE_PM                         0x03
#define FILE_PO                         0x04
#define FILE_PI                         0x08
#define FILE_CP                         0x10
#define FILE_CR                         0x11
#define FILE_CW                         0x12
#define FILE_CM                         0x13
#define FILE_CO                         0x14
#define FILE_CI                         0x18
#define FILE_L0                         0x20
#define FILE_L1                         0x21
#define FILE_L2                         0x22
#define FILE_L3                         0x23
#define FILE_L4                         0x24
#define FILE_L5                         0x25
#define FILE_L6                         0x26
#define FILE_L7                         0x27
#define FILE_L8                         0x28
#define FILE_L9                         0x29
#define FILE_LA                         0x2A
#define FILE_LB                         0x2B
#define FILE_LC                         0x2C
#define FILE_LD                         0x2D
#define FILE_LE                         0x2E
#define FILE_LF                         0x2F

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
