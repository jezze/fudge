struct binary_format
{

    struct resource resource;
    unsigned int (*match)(unsigned long base);
    unsigned long (*findsymbol)(unsigned long base, unsigned int count, char *symbol);
    unsigned long (*findentry)(unsigned long base);
    unsigned int (*map)(unsigned long base, unsigned long paddress, struct mmap_header *mheader);

};

struct binary_format *binary_findformat(unsigned long base);
void binary_initformat(struct binary_format *format, unsigned int (*match)(unsigned long base), unsigned long (*findsymbol)(unsigned long base, unsigned int count, char *symbol), unsigned long (*findentry)(unsigned long base), unsigned int (*map)(unsigned long base, unsigned long paddress, struct mmap_header *mheader));
