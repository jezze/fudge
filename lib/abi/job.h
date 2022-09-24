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

unsigned int job_parse(struct job *jobs, unsigned int c, unsigned int n, void *buffer, unsigned int count);
unsigned int job_spawn(struct job *jobs, unsigned int n);
void job_pipe(struct job *jobs, unsigned int n);
unsigned int job_run(struct job *jobs, unsigned int n);
unsigned int job_close(unsigned int id, struct job *jobs, unsigned int n);
void job_send(struct job *jobs, unsigned int n, unsigned int event, unsigned int count, void *buffer);
void job_kill(struct job *jobs, unsigned int n);
unsigned int job_count(struct job *jobs, unsigned int n);
