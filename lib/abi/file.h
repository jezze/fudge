#define FILE_PP                         0x00
#define FILE_PR                         0x01
#define FILE_PW                         0x02
#define FILE_PM                         0x03
#define FILE_P0                         0x04
#define FILE_P1                         0x05
#define FILE_P2                         0x06
#define FILE_P3                         0x07
#define FILE_P4                         0x08
#define FILE_P5                         0x09
#define FILE_P6                         0x0A
#define FILE_P7                         0x0B
#define FILE_P8                         0x0C
#define FILE_P9                         0x0D
#define FILE_PA                         0x0E
#define FILE_PB                         0x0F
#define FILE_CP                         0x10
#define FILE_CR                         0x11
#define FILE_CW                         0x12
#define FILE_CM                         0x13
#define FILE_C0                         0x14
#define FILE_C1                         0x15
#define FILE_C2                         0x16
#define FILE_C3                         0x17
#define FILE_C4                         0x18
#define FILE_C5                         0x19
#define FILE_C6                         0x1A
#define FILE_C7                         0x1B
#define FILE_C8                         0x1C
#define FILE_C9                         0x1D
#define FILE_CA                         0x1E
#define FILE_CB                         0x1F
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
#define FILE_G0                         0x30
#define FILE_G1                         0x31
#define FILE_G2                         0x32
#define FILE_G3                         0x33
#define FILE_G4                         0x34
#define FILE_G5                         0x35
#define FILE_G6                         0x36
#define FILE_G7                         0x37
#define FILE_G8                         0x38
#define FILE_G9                         0x39
#define FILE_GA                         0x3A
#define FILE_GB                         0x3B
#define FILE_GC                         0x3C
#define FILE_GD                         0x3D
#define FILE_GE                         0x3E
#define FILE_GF                         0x3F

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
