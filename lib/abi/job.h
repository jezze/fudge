struct job
{

    unsigned int id;
    unsigned int ninputs;
    unsigned int ndatas;
    void *data[32];

};

struct job_proc
{

    char *path;
    unsigned int id;
    char *inputs[32];
    unsigned int ninputs;
    char *files[32];
    unsigned int nfiles;

};

void job_interpret(struct job *jobs, unsigned int njobs, struct channel *channel, void *buffer, unsigned int count);
unsigned int job_parse(struct job_proc *procs, void *buffer, unsigned int count);
void job_run(struct channel *channel, struct job_proc *procs, unsigned int n);
