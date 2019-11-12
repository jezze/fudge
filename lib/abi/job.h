struct job_status
{

    char *start;
    char *end;

};

struct job
{

    char *path;
    unsigned int id;
    char *inputs[32];
    unsigned int ninputs;
    char *files[32];
    unsigned int nfiles;
    char *redirects[32];
    unsigned int nredirects;

};

unsigned int job_parse(struct job_status *status, struct job *jobs, unsigned int n);
void job_run(struct channel *channel, struct job *jobs, unsigned int n);
