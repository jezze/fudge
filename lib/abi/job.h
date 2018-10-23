struct job
{

    unsigned int id;
    unsigned int ninputs;

};

void job_run(struct event_header *iheader, struct event_header *oheader, void *buffer, unsigned int count);
