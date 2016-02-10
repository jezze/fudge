struct binary_node
{

    unsigned long physical;

};

struct binary_format
{

    struct resource resource;
    unsigned int (*match)(struct binary_node *node);
    unsigned long (*findsymbol)(struct binary_node *node, unsigned int count, char *symbol);
    unsigned long (*findentry)(struct binary_node *node);
    unsigned long (*findbase)(struct binary_node *node, unsigned long address);
    unsigned int (*copyprogram)(struct binary_node *node);
    unsigned int (*relocate)(struct binary_node *node, unsigned int address);

};

struct binary_format *binary_findformat(struct binary_node *node);
void binary_initformat(struct binary_format *format, unsigned int (*match)(struct binary_node *node), unsigned long (*findsymbol)(struct binary_node *node, unsigned int count, char *symbol), unsigned long (*findentry)(struct binary_node *node), unsigned long (*findbase)(struct binary_node *node, unsigned long address), unsigned int (*copyprogram)(struct binary_node *node), unsigned int (*relocate)(struct binary_node *node, unsigned int address));
void binary_setupelf(void);
