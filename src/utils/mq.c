#include <fudge.h>
#include <abi.h>

#define INPUT_SIZE                      8192
#define PIECE_KEY                       1
#define PIECE_INDEX                     2
#define PIECE_SELF                      3
#define PIECE_FILTER                    4

struct piece
{

    unsigned int type;
    unsigned int index;
    char *name;
    unsigned int len;
    char *value;
    unsigned int vlen;

};

struct parser
{

    char *pos;
    unsigned int source;

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

static void parse_bracket(struct parser *ps, struct piece *piece)
{

    char *start;

    next(ps);

    start = ps->pos;

    while (!is_path_end(ps) && *ps->pos != ']' && *ps->pos != ':')
        next(ps);

    if (*ps->pos == ':')
    {

        piece->type = PIECE_FILTER;
        piece->name = start;
        piece->len = ps->pos - start;

        next(ps);

        piece->value = ps->pos;

        while (!is_path_end(ps) && *ps->pos != ']')
            next(ps);

        piece->vlen = ps->pos - piece->value;

    }

    else
    {

        piece->type = PIECE_INDEX;
        piece->index = cstring_read_value(start, ps->pos - start, 10);

    }

    if (!is_path_end(ps))
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
        parse_bracket(ps, piece);
    else
        parse_key(ps, piece);

}

static unsigned int getvalue(struct parser *ps, char **start)
{

    unsigned int quoted = is_quote(ps);
    unsigned int length;

    *start = ps->pos;

    skip_value(ps);

    length = ps->pos - *start;

    if (quoted)
    {

        *start = *start + 1;
        length -= 2;

    }

    return length;

}

static unsigned int matchfilter(struct parser *ps, struct piece *piece)
{

    struct parser element = *ps;
    char *start;
    unsigned int length;

    if (!is_object_start(&element) || !find_key(&element, piece))
        return 0;

    length = getvalue(&element, &start);

    return length == piece->vlen && buffer_match(start, piece->value, length);

}

static void output(struct parser *ps, char *format)
{

    char line[MESSAGE_SIZE];
    unsigned int c = 0;
    char *start;
    unsigned int length;

    if (!*format)
    {

        length = getvalue(ps, &start);

        channel_send_fmt(0, ps->source, EVENT_DATA, "%w\n", start, &length);

        return;

    }

    if (is_array_start(ps))
    {

        next(ps);
        skip_separators(ps);

        while (!is_array_end(ps))
        {

            struct parser element = *ps;

            output(&element, format);
            skip_value(ps);
            skip_separators(ps);

        }

        return;

    }

    while (*format)
    {

        char *end = format + 1;

        while (*format == '{' && *end && *end != '}')
            end++;

        if (*format == '{' && *end == '}')
        {

            struct parser field = *ps;
            struct piece piece;

            piece.name = format + 1;
            piece.len = end - format - 1;

            if (is_object_start(&field) && find_key(&field, &piece))
            {

                length = getvalue(&field, &start);
                c += buffer_write(line, MESSAGE_SIZE - 1, start, length, c);

            }

            format = end + 1;

        }

        else
        {

            c += buffer_write(line, MESSAGE_SIZE - 1, format, 1, c);
            format++;

        }

    }

    line[c] = '\n';

    channel_send(0, ps->source, EVENT_DATA, c + 1, line);

}

static unsigned int walk(struct parser *ps, struct parser *path);

static unsigned int walk_key_in_array(struct parser *ps, struct parser *path)
{

    struct parser element;
    unsigned int count = 0;

    next(ps);
    skip_separators(ps);

    while (!is_array_end(ps))
    {

        element = *ps;
        count += walk(&element, path);

        skip_value(ps);
        skip_separators(ps);

    }

    return count;

}

static unsigned int walk_index(struct parser *ps, struct piece *piece, struct parser *rest)
{

    if (!is_array_start(ps))
        return 0;

    if (!find_element(ps, piece))
        return 0;

    return walk(ps, rest);

}

static unsigned int matchfilters(struct parser *ps, struct parser *path)
{

    struct parser filters = *path;
    struct piece piece;

    while (is_path_index_start(&filters))
    {

        parse_piece(&filters, &piece);

        if (piece.type != PIECE_FILTER)
            return 1;

        if (!matchfilter(ps, &piece))
            return 0;

    }

    return 1;

}

static unsigned int walk_filter(struct parser *ps, struct parser *path)
{

    struct parser rest = *path;
    struct parser after = *path;
    struct piece piece;
    unsigned int matches = 0;
    unsigned int count = 0;

    if (!is_array_start(ps))
        return 0;

    piece.type = PIECE_FILTER;

    while (is_path_index_start(&after) && piece.type == PIECE_FILTER)
    {

        rest = after;

        parse_piece(&after, &piece);

    }

    if (piece.type == PIECE_FILTER)
        rest = after;

    next(ps);
    skip_separators(ps);

    while (!is_array_end(ps))
    {

        if (matchfilters(ps, path))
        {

            struct parser element = *ps;

            if (piece.type != PIECE_INDEX)
                count += walk(&element, &rest);
            else if (matches++ == piece.index)
                return walk(&element, &after);

        }

        skip_value(ps);
        skip_separators(ps);

    }

    return count;

}

static unsigned int walk_key_in_object(struct parser *ps, struct piece *piece, struct parser *rest)
{

    if (!find_key(ps, piece))
        return 0;

    return walk(ps, rest);

}

static unsigned int walk_key(struct parser *ps, struct piece *piece, struct parser *path, struct parser *rest)
{

    if (is_object_start(ps))
        return walk_key_in_object(ps, piece, rest);

    if (is_array_start(ps))
        return walk_key_in_array(ps, path);

    return 0;

}

static unsigned int walk(struct parser *ps, struct parser *path)
{

    struct parser rest = *path;
    struct piece piece;

    if (is_path_end(path))
    {

        output(ps, option_getstring("format"));

        return 1;

    }

    parse_piece(&rest, &piece);

    switch (piece.type)
    {

    case PIECE_INDEX:
        return walk_index(ps, &piece, &rest);

    case PIECE_KEY:
        return walk_key(ps, &piece, path, &rest);

    case PIECE_SELF:
        return walk(ps, &rest);

    case PIECE_FILTER:
        return walk_filter(ps, path);

    default:
        return 0;

    }

}

static void parse(unsigned int source, char *input, char *query)
{

    struct parser path;
    struct parser doc;

    doc.pos = input;
    doc.source = source;
    path.pos = query;
    path.source = source;

    skip_separators(&doc);

    if (!walk(&doc, &path))
        channel_send_fmt(0, source, EVENT_ERROR, "Key not found: %s\n", query);

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
            unsigned int count = fs_read_full(1, target, id, buffer, INPUT_SIZE - 1, 0);

            if (count)
            {

                buffer[count] = '\0';

                parse(message->source, buffer, option_getstring("query"));

            }

        }

        else
        {

            channel_send_fmt(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

        }

    }

    else
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "Service not found: %s\n", message->data);

    }

}

void init(void)
{

    option_add("query", ".");
    option_add("format", "");
    channel_bind(EVENT_PATH, onpath);

}

