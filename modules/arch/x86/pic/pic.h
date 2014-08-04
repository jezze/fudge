struct pic_routine
{

    struct base_bus *bus;
    unsigned int id;
    void (*callback)(unsigned int irq, struct base_bus *bus, unsigned int id);

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
void pic_enable_line(unsigned short port, unsigned char line);
void pic_disable_line(unsigned short port, unsigned char line);
unsigned int pic_set_routine(struct base_bus *bus, unsigned int id, void (*callback)(unsigned int irq, struct base_bus *bus, unsigned int id));
unsigned int pic_unset_routine(struct base_bus *bus, unsigned int id);
