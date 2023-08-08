struct core
{

    struct resource resource;
    struct list tasks;
    unsigned int id;
    unsigned int sp;
    unsigned int itask;

};

void core_register(struct core *core);
void core_unregister(struct core *core);
void core_init(struct core *core, unsigned int id, unsigned int sp);
