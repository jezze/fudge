struct binary_format
{

    struct resource resource;
    unsigned int (*match)(unsigned int physical);
    unsigned int (*findsymbol)(unsigned int physical, unsigned int count, char *symbol);
    unsigned int (*findentry)(unsigned int physical);
    unsigned int (*findbase)(unsigned int physical, unsigned int address);
    unsigned int (*copyprogram)(unsigned int physical);
    unsigned int (*relocate)(unsigned int physical);

};

struct binary_format *binary_findformat(unsigned int physical);
void binary_initformat(struct binary_format *format, unsigned int (*match)(unsigned int physical), unsigned int (*findsymbol)(unsigned int physical, unsigned int count, char *symbol), unsigned int (*findentry)(unsigned int physical), unsigned int (*findbase)(unsigned int physical, unsigned int address), unsigned int (*copyprogram)(unsigned int physical), unsigned int (*relocate)(unsigned int physical));
void binary_setupelf(void);
