struct parser
{

    struct
    {

        char *data;
        unsigned int count;
        unsigned int offset;
        unsigned int line;
        unsigned int linebreak;
        unsigned int inside;
        unsigned int escaped;

    } expr;

    unsigned int errors;

};

void parser_parse(struct parser *parser, unsigned int source, char *in, unsigned int count, void *data);
