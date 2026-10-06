#define JOB_COMMANDS                    8
#define JOB_PATHS                       16
#define JOB_OPTIONS                     16
#define JOB_STRINGSSIZE                 512
#define JOB_ERRORSIZE                   128

struct job_command
{

    char *program;
    char *paths[JOB_PATHS];
    unsigned int npaths;
    char *keys[JOB_OPTIONS];
    char *values[JOB_OPTIONS];
    unsigned int noptions;
    unsigned int target;

};

struct job
{

    struct job_command commands[JOB_COMMANDS];
    unsigned int ncommands;
    unsigned int background;
    char strings[JOB_STRINGSSIZE];
    unsigned int nstrings;
    char error[JOB_ERRORSIZE];

};

unsigned int job_spawn(struct job *job, unsigned int ichannel, unsigned int notify, char *bindir);
void job_run(struct job *job, unsigned int ichannel, char *pwd);
void job_abort(struct job *job, unsigned int ichannel);
void job_detach(struct job *job);
unsigned int job_exist(struct job *job, unsigned int target);
unsigned int job_exit(struct job *job, unsigned int ichannel, unsigned int target, unsigned int status);
void job_kill(struct job *job);
void job_sendfirst(struct job *job, unsigned int ichannel, unsigned int event, unsigned int count, void *buffer);
void job_sendall(struct job *job, unsigned int ichannel, unsigned int event, unsigned int count, void *buffer);
unsigned int job_count(struct job *job);
