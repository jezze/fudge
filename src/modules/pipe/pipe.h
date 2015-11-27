struct pipe_end
{

    struct system_node node;
    unsigned char data[4096];
    struct buffer buffer;
    struct list readlist;
    struct list writelist;

};

struct pipe
{

    struct system_node root;
    struct pipe_end end0;
    struct pipe_end end1;

};

void pipe_init(struct pipe *pipe);
void pipe_register(struct pipe *pipe);
void pipe_unregister(struct pipe *pipe);
