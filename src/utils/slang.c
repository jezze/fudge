#include <abi.h>
#include <fudge.h>

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

struct task
{

    unsigned int id;
    unsigned int ninputs;
    unsigned int noutputs;

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

static unsigned int add(struct task *task, unsigned int count, unsigned int id)
{

    if (id)
    {

        task[count].id = id;

        return count + 1;

    }

    return count;

}

static unsigned int clear(struct task *task, unsigned int count)
{

    memory_clear(task, sizeof (struct task) * count);

    return 0;

}

static void run(struct event_header *header, struct task *task, unsigned int count)
{

    char message[FUDGE_BSIZE];
    unsigned int j;
    unsigned int k;
    unsigned int x;

    for (j = 0; j < count; j++)
    {

        event_addrequest(message, header, EVENT_INIT, task[j].id);
        event_send(message);

    }

    for (j = 0; j < count; j++)
    {

        for (k = 0; k < task[j].ninputs; k++)
        {

            event_addpipe(message, header, EVENT_FILE, task[j].id);

            for (x = count; x > j + 1; x--)
                event_addforward(message, task[x - 1].id);

            event_addfile(message, FILE_PI + k);
            event_send(message);

        }

    }

    if (!task[0].ninputs)
    {

        event_addpipe(message, header, EVENT_DATA, task[0].id);

        for (x = count; x > 0 + 1; x--)
            event_addforward(message, task[x - 1].id);

        event_adddata(message, 0, 0);
        event_send(message);

    }

    for (j = 0; j < count; j++)
    {

        event_addrequest(message, header, EVENT_EXIT, task[j].id);
        event_send(message);

    }

}

static void parse(struct event_header *header, struct tokenlist *postfix, struct tokenlist *stack)
{

    struct task task[32];
    unsigned int ntask = clear(task, 32);
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

            if (!file_walk(FILE_CI + task[ntask].ninputs, t->str))
                return;

            task[ntask].ninputs++;

            break;

        case TOKENOUT:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!file_walk(FILE_CO, t->str))
                return;

            task[ntask].noutputs++;

            break;

        case TOKENPIPE:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!(file_walkfrom(FILE_CP, FILE_L0, t->str) || file_walk(FILE_CP, t->str)))
                return;

            ntask = add(task, ntask, call_spawn());

            break;

        case TOKENEND:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!(file_walkfrom(FILE_CP, FILE_L0, t->str) || file_walk(FILE_CP, t->str)))
                return;

            ntask = add(task, ntask, call_spawn());

            run(header, task, ntask);

            ntask = clear(task, ntask);

            break;

        }

    }

}

static void oninit(struct event_header *header)
{

    if (!file_walk(FILE_L0, "/bin"))
        return;

    ring_init(&stringtable, FUDGE_BSIZE, stringdata);
    tokenlist_init(&infix, 1024, infixdata);
    tokenlist_init(&postfix, 1024, postfixdata);
    tokenlist_init(&stack, 8, stackdata);

}

static void onkill(struct event_header *header)
{

    char message[FUDGE_BSIZE];

    event_addresponse(message, header, EVENT_CHILD);
    event_send(message);

    quit = 1;

}

static void ondata(struct event_header *header)
{

    struct event_data *data = event_getdata(header);

    if (!data->count)
        return;

    tokenizebuffer(&infix, &stringtable, data->count, data + 1);
    translate(&postfix, &infix, &stack);
    parse(header, &postfix, &stack);

}

static void onfile(struct event_header *header)
{

    struct event_file *file = event_getdata(header);
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        tokenizebuffer(&infix, &stringtable, count, buffer);

    file_close(file->descriptor);
    translate(&postfix, &infix, &stack);
    parse(header, &postfix, &stack);

}

void main(void)
{

    event_open();

    while (!quit)
    {

        char data[FUDGE_BSIZE];
        struct event_header *header = event_read(data);

        switch (header->type)
        {

        case EVENT_INIT:
            oninit(header);

            break;

        case EVENT_EXIT:
        case EVENT_KILL:
            onkill(header);

            break;

        case EVENT_DATA:
            ondata(header);

            break;

        case EVENT_FILE:
            onfile(header);

            break;

        }

    }

    event_close();

}

