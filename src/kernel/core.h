struct core
{

    unsigned int id;
    unsigned int sp;
    struct resource resource;
    struct list tasks;

};

void core_init(struct core *core, unsigned int id, unsigned int sp);
