#include <fudge.h>
#include <abi.h>

#define TOKEN_SKIP                      1
#define TOKEN_END                       2
#define TOKEN_IDENT                     3
#define TOKEN_OPTION                    4
#define TOKEN_PIPE                      5

struct token
{

    unsigned int type;
    char *str;

};

struct tokenlist
{

    unsigned int head;
    unsigned int size;
    struct token *table;

};

static void tokenlist_init(struct tokenlist *list, unsigned int size, struct token *table)
{

    list->head = 0;
    list->size = size;
    list->table = table;

}

static void tokenlist_push(struct tokenlist *list, struct token *token)
{

    list->table[list->head].type = token->type;
    list->table[list->head].str = token->str;

    if (list->head < list->size)
        list->head++;

}

static struct token *tokenlist_pop(struct tokenlist *list)
{

    return (list->head) ? &list->table[--list->head] : 0;

}

static unsigned int tokenlist_check(struct tokenlist *list)
{

    return (list->head) ? list->table[list->head - 1].type : 0;

}

static void tokenlist_add(struct tokenlist *list, unsigned int type, char *str)
{

    struct token token;

    token.type = type;
    token.str = str;

    tokenlist_push(list, &token);

}

static unsigned int precedence(struct token *token)
{

    switch (token->type)
    {

    case TOKEN_END:
        return 1;

    case TOKEN_PIPE:
        return 2;

    case TOKEN_OPTION:
        return 3;

    }

    return 0;

}

static unsigned int tokenize(char c)
{

    switch (c)
    {

    case ' ':
    case '\t':
        return TOKEN_SKIP;

    case '?':
        return TOKEN_OPTION;

    case '|':
        return TOKEN_PIPE;

    case ';':
    case '\n':
        return TOKEN_END;

    }

    return TOKEN_IDENT;

}

static unsigned int getidentlength(unsigned int count, char *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int token = tokenize(buffer[i]);

        if (token != TOKEN_IDENT)
            break;

    }

    return i;

}

static void tokenizebuffer(struct tokenlist *infix, struct ring *stringtable, unsigned int count, void *buffer)
{

    char *data = buffer;
    unsigned int i;
    unsigned int c;

    for (i = 0; i < count; i++)
    {

        unsigned int token = tokenize(data[i]);

        switch (token)
        {

        case TOKEN_SKIP:
            continue;

        case TOKEN_IDENT:
            tokenlist_add(infix, token, stringtable->buffer + ring_count(stringtable));

            c = getidentlength(count - i, data + i);

            ring_write(stringtable, &data[i], c);
            ring_write(stringtable, "\0", 1);

            i += c - 1;

            break;

        default:
            tokenlist_add(infix, token, 0);

            break;

        }

    }

}

static void translate(struct tokenlist *postfix, struct tokenlist *infix, struct tokenlist *stack)
{

    unsigned int i;

    for (i = 0; i < infix->head; i++)
    {

        struct token *token = &infix->table[i];
        struct token *t;

        if (token->type == TOKEN_IDENT)
        {

            tokenlist_push(postfix, token);

            continue;

        }

        if (token->type == TOKEN_END || token->type == TOKEN_PIPE)
        {

            while ((t = tokenlist_pop(stack)))
                tokenlist_push(postfix, t);

            tokenlist_push(postfix, token);

            continue;

        }

        while ((t = tokenlist_pop(stack)))
        {

            if (precedence(token) > precedence(t))
            {

                tokenlist_push(stack, t);

                break;

            }

            tokenlist_push(postfix, t);

        }

        tokenlist_push(stack, token);

    }

}

static void parse(unsigned int source, struct tokenlist *postfix, struct tokenlist *stack, unsigned int size, void *buffer)
{

    unsigned int count = 0;
    unsigned int i;

    for (i = 0; i < postfix->head; i++)
    {

        struct token *token = &postfix->table[i];
        struct token *t;
        struct token *u;

        switch (token->type)
        {

        case TOKEN_IDENT:
            tokenlist_push(stack, token);

            break;

        case TOKEN_OPTION:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            u = tokenlist_pop(stack);

            if (!u)
                return;

            count += cstring_write_fmt2(buffer, size, count, "O\\0%s\\0%s\\0", t->str, u->str);

            break;

        case TOKEN_PIPE:
            while (tokenlist_check(stack) == TOKEN_IDENT)
            {

                t = tokenlist_pop(stack);

                if (tokenlist_check(stack) == TOKEN_IDENT)
                    count += cstring_write_fmt1(buffer, size, count, "D\\0%s\\0", t->str);
                else
                    count += cstring_write_fmt1(buffer, size, count, "P\\0%s\\0", t->str);

            }

            break;

        case TOKEN_END:
            while (tokenlist_check(stack) == TOKEN_IDENT)
            {

                t = tokenlist_pop(stack);

                if (tokenlist_check(stack) == TOKEN_IDENT)
                    count += cstring_write_fmt1(buffer, size, count, "D\\0%s\\0", t->str);
                else
                    count += cstring_write_fmt1(buffer, size, count, "P\\0%s\\0", t->str);

            }

            count += cstring_write_fmt0(buffer, size, count, "E\\0");

            channel_send_buffer(0, source, EVENT_DATA, count, buffer);

            count = 0;

            break;

        }

    }

}

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    if (msize)
    {

        char buffer[1024];
        char stringdata[1024];
        struct ring stringtable;
        struct token infixdata[1024];
        struct token postfixdata[1024];
        struct token stackdata[8];
        struct tokenlist infix;
        struct tokenlist postfix;
        struct tokenlist stack;

        ring_init(&stringtable, 1024, stringdata);
        tokenlist_init(&infix, 1024, infixdata);
        tokenlist_init(&postfix, 1024, postfixdata);
        tokenlist_init(&stack, 8, stackdata);
        tokenizebuffer(&infix, &stringtable, msize, mdata);
        translate(&postfix, &infix, &stack);
        parse(source, &postfix, &stack, 1024, buffer);

    }

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);

}

