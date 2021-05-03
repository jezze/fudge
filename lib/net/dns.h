struct dns_header
{

    unsigned char id[2];
    unsigned char flags[2];
    unsigned char questions[2];
    unsigned char answers[2];
    unsigned char authority[2];
    unsigned char additional[2];

};

struct dns_question
{

    unsigned char type[2];
    unsigned char class[2];

};

struct dns_answer
{

    unsigned char type[2];
    unsigned char class[2];
    unsigned char ttl[4];
    unsigned char rdlength[2];

};

unsigned int dns_namesize(char *name);
unsigned int dns_copyname(void *buffer, unsigned int count, char *name);
unsigned int dns_writename(void *buffer, unsigned int count, char *name, void *start);
