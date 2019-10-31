struct job_status
{

    char *start;
    char *end;

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

unsigned int job_parse(struct job_status *status, struct job_proc *procs, unsigned int n);
void job_run(struct channel *channel, struct job_proc *procs, unsigned int n);
