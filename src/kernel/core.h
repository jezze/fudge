struct core
{

    struct resource resource;
    struct list_item item;
    struct list tasks;
    unsigned int id;
    unsigned int sp;
    struct task *task;

};

struct task *core_schedule(struct core *core);
struct task *core_unschedule(struct core *core);
void core_register(struct core *core);
void core_unregister(struct core *core);
void core_init(struct core *core, unsigned int id, unsigned int sp, struct task *task);
