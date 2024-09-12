struct binary_format
{

    struct resource resource;
    unsigned int (*match)(unsigned int base);
    unsigned int (*findsymbol)(unsigned int base, unsigned int count, char *symbol);
    unsigned int (*findentry)(unsigned int base);
    unsigned int (*findbase)(unsigned int base, unsigned int address);
    unsigned int (*copyprogram)(unsigned int base);
    void (*relocate)(unsigned int base);

};

struct binary_format *binary_findformat(unsigned int base);
void binary_initformat(struct binary_format *format, unsigned int (*match)(unsigned int base), unsigned int (*findsymbol)(unsigned int base, unsigned int count, char *symbol), unsigned int (*findentry)(unsigned int base), unsigned int (*findbase)(unsigned int base, unsigned int address), unsigned int (*copyprogram)(unsigned int base), void (*relocate)(unsigned int base));
