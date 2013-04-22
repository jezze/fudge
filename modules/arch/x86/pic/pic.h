struct pic_registers
{

    struct {unsigned int edi; unsigned int esi; unsigned int ebp; unsigned int esp; unsigned int ebx; unsigned int edx; unsigned int ecx; unsigned int eax;} general;
    unsigned int index;
    unsigned int slave;
    struct {unsigned int eip; unsigned int code; unsigned int eflags; unsigned int esp; unsigned int data;} interrupt;

};

struct pic_routine
{

    struct base_device *device;
    void (*callback)(struct base_device *device);

};

void pic_routine00();
void pic_routine01();
void pic_routine02();
void pic_routine03();
void pic_routine04();
void pic_routine05();
void pic_routine06();
void pic_routine07();
void pic_routine08();
void pic_routine09();
void pic_routine0A();
void pic_routine0B();
void pic_routine0C();
void pic_routine0D();
void pic_routine0E();
void pic_routine0F();
unsigned short pic_interrupt(struct pic_registers *registers);
void pic_set_mask(unsigned short port, unsigned char mask);
void pic_enable_line(unsigned short port, unsigned char line);
void pic_disable_line(unsigned short port, unsigned char line);
unsigned int pic_set_routine(unsigned int index, struct base_device *device, void (*callback)(struct base_device *device));
unsigned int pic_unset_routine(unsigned int index, struct base_device *device);
