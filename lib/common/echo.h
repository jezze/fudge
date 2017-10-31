struct echo_buf
{

    void *data;
    unsigned int length;

};

union echo_arg
{

    char *s;
    int d;
    struct echo_buf b;

};

void echo(unsigned int descriptor, char *s, union echo_arg *a);
