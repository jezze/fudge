#include <fudge.h>
#include <abi.h>

#define INPUT_SIZE    (8 * 1024)
#define PIECE_KEY    0
#define PIECE_INDEX    1
#define PIECE_SELF    2

struct piece
{

    unsigned int type;
    unsigned int index;
    char *name;
    unsigned int len;
};

struct parser
{

    char *pos;

};

static void next(struct parser *ps)
{

    ps->pos++;

}

static int is_separator(struct parser *ps)
{

    switch (*ps->pos)
    {

    case ' ':
    case '\t':
    case '\n':
    case ',':
        return 1;

    default:
        return 0;

    }

}

static int is_scalar_end(struct parser *ps)
{

    switch (*ps->pos)
    {

    case ' ':
    case '\t':
    case '\n':
    case ',':
    case '}':
    case ']':
        return 1;

    default:
        return 0;

    }

}

static int is_key_end(struct parser *ps)
{

    return *ps->pos == ':';

}

static int is_quote(struct parser *ps)
{

    return *ps->pos == '"';

}

static int is_object_start(struct parser *ps)
{

    return *ps->pos == '{';

}

static int is_array_start(struct parser *ps)
{

    return *ps->pos == '[';

}

static int is_object_end(struct parser *ps)
{

    return *ps->pos == '}';

}

static int is_array_end(struct parser *ps)
{

    return *ps->pos == ']';

}

static int is_path_end(struct parser *ps)
{

    return *ps->pos == '\0';

}

static int is_path_index_start(struct parser *ps)
{

    return *ps->pos == '[';

}

static int is_path_key_end(struct parser *ps)
{

    switch (*ps->pos)
    {

    case '\0':
    case '.':
    case '[':
    case ']':
        return 1;

    default:
        return 0;

    }

}

static void skip_separators(struct parser *ps)
{

    while (is_separator(ps))
        next(ps);

}

static void skip_string(struct parser *ps)
{

    next(ps);

    while (!is_quote(ps))
        next(ps);

    next(ps);

}

static void skip_scalar(struct parser *ps)
{

    while (!is_scalar_end(ps))
        next(ps);

}

static void skip_key(struct parser *ps)
{

    while (!is_key_end(ps))
        next(ps);

    next(ps);
    skip_separators(ps);

}

static void skip_value(struct parser *ps);

static void skip_object(struct parser *ps)
{

    next(ps);
    skip_separators(ps);

    while (!is_object_end(ps))
    {

        skip_key(ps);
        skip_value(ps);
        skip_separators(ps);

    }

    next(ps);

}

static void skip_array(struct parser *ps)
{

    next(ps);
    skip_separators(ps);

    while (!is_array_end(ps))
    {

        skip_value(ps);
        skip_separators(ps);

    }

    next(ps);

}

static void skip_value(struct parser *ps)
{

    if (is_quote(ps))
        skip_string(ps);
    else if (is_object_start(ps))
        skip_object(ps);
    else if (is_array_start(ps))
        skip_array(ps);
    else
        skip_scalar(ps);

}

static int find_key(struct parser *ps, struct piece *piece)
{

    char *name;
    unsigned int len;

    next(ps);
    skip_separators(ps);

    while (!is_object_end(ps))
    {

        name = ps->pos;
        len = 0;

        while (!is_key_end(ps))
        {

            next(ps);
            len++;

        }

        next(ps);
        skip_separators(ps);

        if (len == piece->len)
        {

            if (buffer_match(name, piece->name, len))
                return 1;

        }

        skip_value(ps);
        skip_separators(ps);

    }

    return 0;

}

static int find_element(struct parser *ps, struct piece *piece)
{

    unsigned int n;

    next(ps);
    skip_separators(ps);

    for (n = 0; n < piece->index; n++)
    {

        if (is_array_end(ps))
            return 0;

        skip_value(ps);
        skip_separators(ps);

    }

    return !is_array_end(ps);

}

static void parse_index(struct parser *ps, struct piece *piece)
{

    piece->type = PIECE_INDEX;
    piece->index = 0;

    next(ps);

    while (!is_path_key_end(ps))
    {

        piece->index = piece->index * 10 + (unsigned int)(*ps->pos - '0');

        next(ps);

    }

    next(ps);

}

static void parse_key(struct parser *ps, struct piece *piece)
{

    piece->type = PIECE_KEY;

    next(ps);

    piece->name = ps->pos;
    piece->len = 0;

    while (!is_path_key_end(ps))
    {

        next(ps);

        piece->len++;

    }

    if (piece->len == 0)
        piece->type = PIECE_SELF;

}

static void parse_piece(struct parser *ps, struct piece *piece)
{

    if (is_path_index_start(ps))
        parse_index(ps, piece);
    else
        parse_key(ps, piece);

}

static unsigned int walk(unsigned int source, struct parser *ps, struct parser *path);

static unsigned int walk_key_in_array(unsigned int source, struct parser *ps, struct parser *path)
{

    struct parser element;
    unsigned int count = 0;

    next(ps);
    skip_separators(ps);

    while (!is_array_end(ps))
    {

        element = *ps;
        count += walk(source, &element, path);

        skip_value(ps);
        skip_separators(ps);

    }

    return count;

}

static unsigned int walk_index(unsigned int source, struct parser *ps, struct piece *piece, struct parser *rest)
{

    if (!is_array_start(ps))
        return 0;

    if (!find_element(ps, piece))
        return 0;

    return walk(source, ps, rest);

}

static unsigned int walk_key_in_object(unsigned int source, struct parser *ps, struct piece *piece, struct parser *rest)
{

    if (!find_key(ps, piece))
        return 0;

    return walk(source, ps, rest);

}

static unsigned int walk_key(unsigned int source, struct parser *ps, struct piece *piece, struct parser *path, struct parser *rest)
{

    if (is_object_start(ps))
        return walk_key_in_object(source, ps, piece, rest);

    if (is_array_start(ps))
        return walk_key_in_array(source, ps, path);

    return 0;

}

static unsigned int walk(unsigned int source, struct parser *ps, struct parser *path)
{

    struct parser rest = *path;
    struct piece piece;
    char *start;

    if (is_path_end(path))
    {

        unsigned int length;

        start = ps->pos;

        skip_value(ps);

        length = ps->pos - start;

        channel_send_fmt2(0, source, EVENT_DATA, "%w\n", start, &length);

        return 1;

    }

    parse_piece(&rest, &piece);

    switch (piece.type)
    {

    case PIECE_INDEX:
        return walk_index(source, ps, &piece, &rest);

    case PIECE_KEY:
        return walk_key(source, ps, &piece, path, &rest);

    case PIECE_SELF:
        return walk(source, ps, &rest);

    default:
        return 0;

    }

}

void parse(unsigned int source, char *input, unsigned int count, char *query)
{

    struct parser path;
    struct parser doc;

    input[count] = '\0';

    doc.pos = input;
    path.pos = query;
    skip_separators(&doc);

    if (!walk(source, &doc, &path))
        channel_send_fmt1(0, source, EVENT_ERROR, "Key not found: %s\n", query);

}

static void onpath(struct message *message)
{

    unsigned int target = fs_auth(message->data);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, message->data);

        if (id)
        {

            char buffer[INPUT_SIZE];
            unsigned int count = fs_read_full(1, target, id, buffer, INPUT_SIZE, 0);

            if (count)
                parse(message->source, buffer, count, option_getstring("query"));

        }

        else
        {

            channel_send_fmt1(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

        }

    }

    else
    {

        channel_send_fmt1(0, message->source, EVENT_ERROR, "Service not found: %s\n", message->data);

    }

}

void init(void)
{

    option_add("query", ".");
    channel_bind(EVENT_PATH, onpath);

    while (channel_process(0));

}

