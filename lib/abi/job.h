struct job_arg
{

    char *key;
    char *value;

};

struct job_worker
{

    char *program;
    unsigned int target;
    char *paths[32];
    unsigned int npaths;
    struct job_arg options[32];
    unsigned int noptions;

};

struct job
{

    struct job_worker *workers;
    unsigned int capacity;
    unsigned int count;

};

void job_parse(struct job *job, void *buffer, unsigned int count);
unsigned int job_spawn(struct job *job, unsigned int ichannel, char *bindir);
unsigned int job_pipe(struct job *job, unsigned int ichannel, unsigned int source, unsigned int event, void *buffer, unsigned int count);
void job_run(struct job *job, unsigned int ichannel, char *pwd);
void job_close(struct job *job, unsigned int ichannel, unsigned int target);
unsigned int job_exist(struct job *job, unsigned int target);
unsigned int job_pick(struct job *job, unsigned int ichannel, struct message *message, unsigned int count, void *data);
void job_sendfirst(struct job *job, unsigned int ichannel, unsigned int event, unsigned int count, void *buffer);
void job_sendall(struct job *job, unsigned int ichannel, unsigned int event, unsigned int count, void *buffer);
void job_killall(struct job *job);
unsigned int job_count(struct job *job);
void job_init(struct job *job, struct job_worker *workers, unsigned int capacity);
