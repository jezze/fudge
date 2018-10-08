struct job
{

    unsigned int id;
    unsigned int ninputs;

};

unsigned int job_runcmd(struct event_header *iheader, struct event_header *oheader, char *command, char *data, unsigned int count, unsigned int session);
void job_run(struct event_header *iheader, struct event_header *oheader, void *buffer, unsigned int count);
