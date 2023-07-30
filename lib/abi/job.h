struct job_arg
{

    char *key;
    char *value;

};

struct job_worker
{

    char *program;
    unsigned int channel;
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
unsigned int job_spawn(struct job *job, unsigned int pdescriptor, unsigned int wdescriptor);
void job_listen(struct job *job, unsigned int event);
void job_pipe(struct job *job, unsigned int event);
void job_run(struct job *job);
void job_close(struct job *job, unsigned int channel);
unsigned int job_exist(struct job *job, unsigned int channel);
unsigned int job_pick(struct job *job, struct message *message, void *data);
void job_sendfirst(struct job *job, unsigned int event, unsigned int count, void *buffer);
void job_sendall(struct job *job, unsigned int event, unsigned int count, void *buffer);
void job_killall(struct job *job);
unsigned int job_count(struct job *job);
void job_init(struct job *job, struct job_worker *workers, unsigned int capacity);
