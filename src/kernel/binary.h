struct binary_format
{

    struct resource resource;
    unsigned int (*match)(unsigned int base);
    unsigned int (*findsymbol)(unsigned int base, unsigned int count, char *symbol);
    unsigned int (*findentry)(unsigned int base);
    unsigned int (*mapsection)(unsigned int base, struct mmap_entry *entry, unsigned int index);

};

struct binary_format *binary_findformat(unsigned int base);
void binary_initformat(struct binary_format *format, unsigned int (*match)(unsigned int base), unsigned int (*findsymbol)(unsigned int base, unsigned int count, char *symbol), unsigned int (*findentry)(unsigned int base), unsigned int (*mapsection)(unsigned int base, struct mmap_entry *entry, unsigned int index));
