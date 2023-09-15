struct binary_node
{

    unsigned int address;

};

struct binary_format
{

    struct resource resource;
    unsigned int (*match)(struct binary_node *node);
    unsigned int (*findsymbol)(struct binary_node *node, unsigned int count, char *symbol);
    unsigned int (*findentry)(struct binary_node *node);
    unsigned int (*findbase)(struct binary_node *node, unsigned int address);
    unsigned int (*copyprogram)(struct binary_node *node);
    void (*relocate)(struct binary_node *node);

};

struct binary_format *binary_findformat(struct binary_node *node);
void binary_initnode(struct binary_node *node);
void binary_initformat(struct binary_format *format, unsigned int (*match)(struct binary_node *node), unsigned int (*findsymbol)(struct binary_node *node, unsigned int count, char *symbol), unsigned int (*findentry)(struct binary_node *node), unsigned int (*findbase)(struct binary_node *node, unsigned int address), unsigned int (*copyprogram)(struct binary_node *node), void (*relocate)(struct binary_node *node));
