struct string
{

    unsigned int index;
    unsigned int count;

};

struct rule
{

    struct string target;
    struct string prereq[8];
    unsigned int prereqs;

};

struct expression
{

    struct rule rule[64];
    unsigned int rules;

};

unsigned int parse(struct token_state *state, struct expression *expression);
