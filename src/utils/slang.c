#include <fudge.h>
#include <abi.h>

#define TOKENSKIP                       1
#define TOKENEND                        2
#define TOKENIDENT                      3
#define TOKENFILE                       4
#define TOKENREDIRECT                   5
#define TOKENDATA                       6
#define TOKENPIPE                       7

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

static char stringdata[FUDGE_BSIZE];
static struct ring stringtable;
static struct token infixdata[1024];
static struct token postfixdata[1024];
static struct token stackdata[8];
static struct tokenlist infix;
static struct tokenlist postfix;
static struct tokenlist stack;

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

    case TOKENEND:
        return 1;

    case TOKENPIPE:
        return 2;

    case TOKENFILE:
    case TOKENREDIRECT:
    case TOKENDATA:
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
        return TOKENSKIP;

    case '<':
        return TOKENFILE;

    case '>':
        return TOKENREDIRECT;

    case '!':
        return TOKENDATA;

    case '|':
        return TOKENPIPE;

    case ';':
    case '\n':
        return TOKENEND;

    }

    return TOKENIDENT;

}

static unsigned int getidentlength(unsigned int count, char *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int token = tokenize(buffer[i]);

        if (token != TOKENIDENT)
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

        case TOKENSKIP:
            continue;

        case TOKENIDENT:
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

        if (token->type == TOKENIDENT)
        {

            tokenlist_push(postfix, token);

            continue;

        }

        if (token->type == TOKENEND || token->type == TOKENPIPE)
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

static void parse(struct channel *channel, unsigned int source, struct tokenlist *postfix, struct tokenlist *stack)
{

    unsigned int i;

    channel_request(channel, EVENT_DATA);

    for (i = 0; i < postfix->head; i++)
    {

        struct token *token = &postfix->table[i];
        struct token *t;

        switch (token->type)
        {

        case TOKENIDENT:
            tokenlist_push(stack, token);

            break;

        case TOKENFILE:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            channel_append(channel, 2, "F");
            channel_append(channel, ascii_length(t->str) + 1, t->str);

            break;

        case TOKENREDIRECT:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            channel_append(channel, 2, "R");
            channel_append(channel, ascii_length(t->str) + 1, t->str);

            break;

        case TOKENDATA:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            channel_append(channel, 2, "D");
            channel_append(channel, ascii_length(t->str) + 1, t->str);

            break;

        case TOKENPIPE:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            channel_append(channel, 2, "P");
            channel_append(channel, ascii_length(t->str) + 1, t->str);

            break;

        case TOKENEND:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            channel_append(channel, 2, "P");
            channel_append(channel, ascii_length(t->str) + 1, t->str);
            channel_append(channel, 2, "E");

            break;

        }

    }

    channel_place(channel, source);

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (!msize)
        return;

    ring_init(&stringtable, FUDGE_BSIZE, stringdata);
    tokenlist_init(&infix, 1024, infixdata);
    tokenlist_init(&postfix, 1024, postfixdata);
    tokenlist_init(&stack, 8, stackdata);
    tokenizebuffer(&infix, &stringtable, msize, mdata);
    translate(&postfix, &infix, &stack);
    parse(channel, source, &postfix, &stack);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[FUDGE_BSIZE];
        unsigned int count;

        ring_init(&stringtable, FUDGE_BSIZE, stringdata);
        tokenlist_init(&infix, 1024, infixdata);
        tokenlist_init(&postfix, 1024, postfixdata);
        tokenlist_init(&stack, 8, stackdata);
        file_open(FILE_L0);

        while ((count = file_read(FILE_L0, buffer, FUDGE_BSIZE)))
            tokenizebuffer(&infix, &stringtable, count, buffer);

        file_close(FILE_L0);
        translate(&postfix, &infix, &stack);
        parse(channel, source, &postfix, &stack);

    }

}

static void onredirect(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_setredirect(channel, redirect->type, redirect->mode, redirect->id, source);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_MAIN, onmain);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen(&channel);

}

