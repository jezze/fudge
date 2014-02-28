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
    struct input in;
    struct output out;

};

struct pipe
{

    struct command command[8];
    unsigned int commands;

};

struct expression
{

    struct pipe pipe[64];
    unsigned int pipes;

};

unsigned int parse(struct token_state *state, struct expression *expression);
