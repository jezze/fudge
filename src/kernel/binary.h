struct binary_section
{

    unsigned int vaddress;
    unsigned int msize;
    unsigned int fsize;
    unsigned int offset;

};

struct binary_format
{

    struct resource resource;
    unsigned int (*match)(unsigned int base);
    unsigned int (*findsymbol)(unsigned int base, unsigned int count, char *symbol);
    unsigned int (*findentry)(unsigned int base);
    unsigned int (*readsection)(unsigned int base, struct binary_section *section, unsigned int index);

};

struct binary_format *binary_findformat(unsigned int base);
void binary_initformat(struct binary_format *format, unsigned int (*match)(unsigned int base), unsigned int (*findsymbol)(unsigned int base, unsigned int count, char *symbol), unsigned int (*findentry)(unsigned int base), unsigned int (*readsection)(unsigned int base, struct binary_section *section, unsigned int index));
