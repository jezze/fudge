struct string
{

    unsigned int index;
    unsigned int count;

};

struct input
{

    struct string index;
    struct string path;
    struct string data;

};

struct output
{

    struct string index;
    struct string path;

};

struct command
{

    struct string binary;
    struct input in[2];
    unsigned int ins;
    struct output out[2];
    unsigned int outs;

};

struct pipe
{

    struct command command[4];
    unsigned int commands;

};

struct expression
{

    struct pipe pipe[32];
    unsigned int pipes;

};

unsigned int parse(struct token_state *state, struct expression *expression);
