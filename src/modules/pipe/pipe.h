struct pipe
{

    struct system_node root;
    struct system_node idata;
    struct system_node odata;
    struct ring ring;
    char data[FUDGE_BSIZE];
    struct spinlock datalock;

};

void pipe_use(struct pipe *pipe);
void pipe_free(struct pipe *pipe);
void pipe_unblock(struct list *states);
void pipe_init(struct pipe *pipe);
void pipe_register(struct pipe *pipe);
void pipe_unregister(struct pipe *pipe);
