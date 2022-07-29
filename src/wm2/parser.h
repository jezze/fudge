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

    void (*fail)(void);
    struct widget *(*find)(char *name);
    struct widget *(*create)(unsigned int type, char *id, char *in);
    struct widget *(*destroy)(struct widget *widget);
    void (*clear)(struct widget *widget);
    unsigned int errors;

};

void parser_parse(struct parser *parser, char *in, unsigned int count, void *data);
void parser_init(struct parser *parser, void (*fail)(void), struct widget *(*find)(char *name), struct widget *(*create)(unsigned int type, char *id, char *in), struct widget *(*destroy)(struct widget *widget), void (*clear)(struct widget *widget));
