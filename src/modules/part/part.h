struct part
{

    struct resource resource;
    struct system_node root;
    struct system_node ctrl;
    struct system_node data;
    struct ctrl_partsettings settings;

};

struct block_interface *part_findinterface(unsigned int index);
void part_init(struct part *part);
void part_register(struct part *part);
void part_unregister(struct part *part);
