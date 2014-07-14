struct rendezvous
{

    struct task *task;

};

unsigned int rendezvous_asleep(struct rendezvous *rendezvous);
void rendezvous_sleep(struct rendezvous *rendezvous, unsigned int condition);
void rendezvous_unsleep(struct rendezvous *rendezvous);
void rendezvous_setup();
