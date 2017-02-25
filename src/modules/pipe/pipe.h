struct pipe_end
{

    struct system_node node;
    struct ring ring;
    struct task_descriptor *read;
    struct task_descriptor *write;
    unsigned int refcount;

};

struct pipe
{

    struct system_node root;
    struct pipe_end end0;
    struct pipe_end end1;

};

void pipe_init(struct pipe *pipe, unsigned int count0, void *data0, unsigned int count1, void *data1);
void pipe_register(struct pipe *pipe);
void pipe_unregister(struct pipe *pipe);
