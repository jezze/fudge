#define CALL_PP                         0x00
#define CALL_PR                         0x01
#define CALL_PW                         0x02
#define CALL_PO                         0x03
#define CALL_PI                         0x04
#define CALL_CP                         0x08
#define CALL_CR                         0x09
#define CALL_CW                         0x0A
#define CALL_CO                         0x0B
#define CALL_CI                         0x0C
#define CALL_L0                         0x10
#define CALL_L1                         0x11
#define CALL_L2                         0x12
#define CALL_L3                         0x13
#define CALL_L4                         0x14
#define CALL_L5                         0x15
#define CALL_L6                         0x16
#define CALL_L7                         0x17
#define CALL_L8                         0x18
#define CALL_L9                         0x19
#define CALL_LA                         0x1A
#define CALL_LB                         0x1B
#define CALL_LC                         0x1C
#define CALL_LD                         0x1D
#define CALL_LE                         0x1E
#define CALL_LF                         0x1F

unsigned int call_auth(unsigned int descriptor, unsigned int backend);
unsigned int call_close(unsigned int descriptor);
unsigned int call_create(unsigned int descriptor, char *name, unsigned int length);
unsigned int call_destroy(unsigned int descriptor, char *name, unsigned int length);
unsigned int call_despawn(void);
unsigned int call_load(unsigned int descriptor);
unsigned int call_mount(unsigned int pdescriptor, unsigned int cdescriptor);
unsigned int call_open(unsigned int descriptor);
unsigned int call_read(unsigned int descriptor, void *buffer, unsigned int count);
unsigned int call_seek(unsigned int descriptor, unsigned int offset);
unsigned int call_spawn(void);
unsigned int call_unload(unsigned int descriptor);
unsigned int call_walk(unsigned int descriptor, unsigned int pdescriptor, char *path, unsigned int length);
unsigned int call_write(unsigned int descriptor, void *buffer, unsigned int count);
