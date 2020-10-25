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

void job_redirect(struct channel *channel, unsigned int target, unsigned int type, unsigned int mode, unsigned int id);
unsigned int job_parse(struct job_status *status, struct job *jobs, unsigned int n);
void job_run(struct channel *channel, struct job *jobs, unsigned int n);
