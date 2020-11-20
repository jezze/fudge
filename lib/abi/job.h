struct job_arg
{

    char *key;
    char *value;

};

struct job
{

    char *path;
    unsigned int id;
    struct job_arg args[32];
    unsigned int nargs;
    char *files[32];
    unsigned int nfiles;

};

unsigned int job_parse(struct job *jobs, unsigned int n, void *buffer, unsigned int count);
void job_replyto(struct channel *channel, unsigned int target, unsigned int type, unsigned int id);
void job_replyback(struct channel *channel, unsigned int target, unsigned int type);
unsigned int job_spawn(struct channel *channel, struct job *jobs, unsigned int n);
unsigned int job_run(struct channel *channel, struct job *jobs, unsigned int n);
unsigned int job_close(struct channel *channel, unsigned int id, struct job *jobs, unsigned int n);
void job_term(struct channel *channel, struct job *jobs, unsigned int n);
void job_kill(struct channel *channel, struct job *jobs, unsigned int n);
unsigned int job_count(struct channel *channel, struct job *jobs, unsigned int n);
