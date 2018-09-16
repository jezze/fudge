#include <abi.h>
#include <fudge.h>
#include <event/base.h>

#define TOKENSKIP                       1
#define TOKENEND                        2
#define TOKENIDENT                      3
#define TOKENIN                         4
#define TOKENOUT                        5
#define TOKENPIPE                       6

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
static unsigned int quit;

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

    if (!list->head)
        return 0;

    list->head--;

    return &list->table[list->head];

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

    case TOKENIN:
    case TOKENOUT:
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
        return TOKENIN;

    case '>':
        return TOKENOUT;

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

static void tokenizebuffer(struct tokenlist *infix, struct ring *stringtable, unsigned int count, char *buffer)
{

    unsigned int i;
    unsigned int c;

    for (i = 0; i < count; i++)
    {

        unsigned int token = tokenize(buffer[i]);

        switch (token)
        {

        case TOKENSKIP:
            continue;

        case TOKENIDENT:
            tokenlist_add(infix, token, stringtable->buffer + ring_count(stringtable));

            c = getidentlength(count - i, buffer + i);

            ring_write(stringtable, &buffer[i], c);
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

static void parse(struct event_header *header, struct tokenlist *postfix, struct tokenlist *stack)
{

    unsigned int rei = 0;
    unsigned int id;
    unsigned int i;

    for (i = 0; i < postfix->head; i++)
    {

        struct token *token = &postfix->table[i];
        struct token *t;

        switch (token->type)
        {

        case TOKENIDENT:
            tokenlist_push(stack, token);

            break;

        case TOKENIN:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!file_walk(FILE_CI, t->str))
                return;

            rei = 1;

            break;

        case TOKENOUT:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!file_walk(FILE_CO, t->str))
                return;

            break;

        case TOKENPIPE:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!(file_walkfrom(FILE_CP, FILE_L1, t->str) || file_walk(FILE_CP, t->str)))
                return;

            id = call_spawn();

            if (!id)
                return;

            event_sendinit(FILE_L0, header->source, id);

            if (rei)
                event_sendfile(FILE_L0, header->source, id, FILE_PI + rei - 1);

            event_sendexit(FILE_L0, header->destination, id);

            rei = 0;

            break;

        case TOKENEND:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!(file_walkfrom(FILE_CP, FILE_L1, t->str) || file_walk(FILE_CP, t->str)))
                return;

            id = call_spawn();

            if (!id)
                return;

            event_sendinit(FILE_L0, header->source, id);

            if (rei)
                event_sendfile(FILE_L0, header->source, id, FILE_PI + rei - 1);

            event_sendexit(FILE_L0, header->destination, id);

            rei = 0;

            break;

        }

    }

}

static void oninit(struct event_header *header, void *data)
{

    if (!file_walk(FILE_L1, "/bin"))
        return;

    ring_init(&stringtable, FUDGE_BSIZE, stringdata);
    tokenlist_init(&infix, 1024, infixdata);
    tokenlist_init(&postfix, 1024, postfixdata);
    tokenlist_init(&stack, 8, stackdata);

}

static void onkill(struct event_header *header, void *data)
{

    event_sendchild(FILE_L0, header->destination, header->source);

    quit = 1;

}

static void ondata(struct event_header *header, void *data)
{

    tokenizebuffer(&infix, &stringtable, header->length - sizeof (struct event_header), data);
    translate(&postfix, &infix, &stack);
    parse(header, &postfix, &stack);

}

static void onfile(struct event_header *header, void *data)
{

    struct event_file *file = data;
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->num);

    while ((count = file_read(file->num, buffer, FUDGE_BSIZE)))
        tokenizebuffer(&infix, &stringtable, count, buffer);

    file_close(file->num);
    translate(&postfix, &infix, &stack);
    parse(header, &postfix, &stack);

}

void main(void)
{

    /* Once event system is inside kernel, there is no need to use the filesystem to send events */
    file_walk(FILE_L0, "/system/event");
    file_open(FILE_L0);

    while (!quit)
    {

        struct event event;

        event_read(FILE_L0, &event);

        switch (event.header.type)
        {

        case EVENT_INIT:
            oninit(&event.header, event.data);

            break;

        case EVENT_EXIT:
        case EVENT_KILL:
            onkill(&event.header, event.data);

            break;

        case EVENT_DATA:
            ondata(&event.header, event.data);

            break;

        case EVENT_FILE:
            onfile(&event.header, event.data);

            break;

        }

    }

    file_close(FILE_L0);

}

