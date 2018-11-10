struct job
{

    unsigned int id;
    unsigned int ninputs;
    unsigned int ndatas;
    void *data[32];

};

void job_interpret(struct job *jobs, unsigned int njobs, union event_message *imessage, union event_message *omessage, void *buffer, unsigned int count, unsigned int session);
