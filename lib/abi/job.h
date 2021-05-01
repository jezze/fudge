struct job_arg
{

    char *key;
    char *value;

};

struct job
{

    char *program;
    unsigned int id;
    char *paths[32];
    unsigned int npaths;
    struct job_arg options[32];
    unsigned int noptions;

};

unsigned int job_parse(struct job *jobs, unsigned int n, void *buffer, unsigned int count);
unsigned int job_spawn(struct channel *channel, struct job *jobs, unsigned int n);
void job_pipe(struct channel *channel, struct job *jobs, unsigned int n);
unsigned int job_run(struct channel *channel, struct job *jobs, unsigned int n);
unsigned int job_close(struct channel *channel, unsigned int id, struct job *jobs, unsigned int n);
void job_send(struct channel *channel, struct job *jobs, unsigned int n, unsigned int event, unsigned int count, void *buffer);
void job_sendall(struct channel *channel, struct job *jobs, unsigned int n, unsigned int event, unsigned int count, void *buffer);
void job_kill(struct channel *channel, struct job *jobs, unsigned int n);
unsigned int job_count(struct channel *channel, struct job *jobs, unsigned int n);
