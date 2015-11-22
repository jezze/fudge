struct pipe
{

    struct system_node root;
    struct system_node p0;
    struct system_node p1;
    unsigned char data0[4096];
    struct buffer buffer0;
    unsigned char data1[4096];
    struct buffer buffer1;
    unsigned int ref0;
    unsigned int ref1;

};

void pipe_init(struct pipe *pipe);
void pipe_register(struct pipe *pipe);
void pipe_unregister(struct pipe *pipe);
