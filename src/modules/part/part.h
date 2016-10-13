struct part
{

    struct system_node root;
    struct system_node ctrl;
    struct ctrl_partsettings settings;
    struct block_interface *parent;
    unsigned int start;
    unsigned int end;

};

void part_init(struct part *part);
void part_register(struct part *part);
void part_unregister(struct part *part);
