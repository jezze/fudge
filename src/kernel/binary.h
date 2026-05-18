struct binary_format
{

    struct resource resource;
    unsigned int (*match)(unsigned long base);
    unsigned long (*findsymbol)(unsigned long base, unsigned int count, char *symbol);
    unsigned long (*findentry)(unsigned long base);
    unsigned int (*mapsection)(unsigned long base, struct mmap_entry *entry, unsigned int index);

};

struct binary_format *binary_findformat(unsigned long base);
void binary_initformat(struct binary_format *format, unsigned int (*match)(unsigned long base), unsigned long (*findsymbol)(unsigned long base, unsigned int count, char *symbol), unsigned long (*findentry)(unsigned long base), unsigned int (*mapsection)(unsigned long base, struct mmap_entry *entry, unsigned int index));
