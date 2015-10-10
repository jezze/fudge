struct pipe
{

    struct system_node root;
    struct system_node p0;
    struct system_node p1;

};

void pipe_init(struct pipe *pipe);
void pipe_register(struct pipe *pipe);
void pipe_unregister(struct pipe *pipe);
