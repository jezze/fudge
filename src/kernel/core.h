#define CORE_STATE_DEAD                 1
#define CORE_STATE_ACTIVE               2

struct core
{

    struct resource resource;
    unsigned int state;
    struct list tasks;
    unsigned int itask;

};

void core_register(struct core *core);
void core_unregister(struct core *core);
void core_init(struct core *core);
