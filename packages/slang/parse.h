struct string
{

    unsigned int index;
    unsigned int count;

};

struct input
{

    struct string path;
    struct string data;

};

struct output
{

    struct string path;

};

struct command
{

    struct string binary;
    struct input in0;
    struct input in1;
    struct output out0;

};

struct pipe
{

    struct command commands[8];
    unsigned int count;

};

struct expression
{

    struct pipe pipes[64];
    unsigned int count;

};

unsigned int parse(struct token_state *state, struct expression *expression);
