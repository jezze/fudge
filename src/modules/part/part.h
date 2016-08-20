struct part
{

    struct system_node root;
    struct system_node data;
    struct block_interface *interface;
    unsigned int offset;

};

void part_init(struct part *part);
void part_register(struct part *part);
void part_unregister(struct part *part);
