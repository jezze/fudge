#include <fudge.h>
#include <abi.h>

#define TEXTSIZE                        0x8000
#define YANKSIZE                        (TEXTSIZE + 1)
#define LINESIZE                        128
#define MESSAGESIZE                     256
#define STATUSSIZE                      256
#define PATHSIZE                        1024
#define TABSIZE                         8
#define RULERSIZE                       16
#define COLUMNEND                       0xFFFFFFFF
#define NOMOTION                        0xFFFFFFFF
#define MODE_NORMAL                     0
#define MODE_INSERT                     1
#define MODE_COMMAND                    2
#define MODE_SEARCH                     3
#define REDRAW_NONE                     0
#define REDRAW_LINE                     1
#define REDRAW_ALL                      2
#define KEY_NONE                        0x100
#define KEY_UP                          0x101
#define KEY_DOWN                        0x102
#define KEY_RIGHT                       0x103
#define KEY_LEFT                        0x104
#define KEY_HOME                        0x105
#define KEY_END                         0x106
#define KEY_DELETE                      0x107
#define KEY_PAGEUP                      0x108
#define KEY_PAGEDOWN                    0x109
#define KEY_GG                          0x10A

struct motion
{

    unsigned int position;
    unsigned int linewise;
    unsigned int inclusive;

};

static unsigned char text[TEXTSIZE];
static unsigned int textcount;
static unsigned char undotext[TEXTSIZE];
static unsigned int undocount;
static unsigned int undocursor;
static unsigned int hasundo;
static unsigned char yank[YANKSIZE];
static unsigned int yankcount;
static unsigned int yanklines;
static char line[LINESIZE];
static unsigned int linecount;
static char search[LINESIZE];
static unsigned int searchcount;
static unsigned int searchforward;
static char notice[MESSAGESIZE];
static char path[PATHSIZE];
static char outdata[MESSAGE_SIZE];
static unsigned int outcount;
static unsigned int output;
static unsigned int rows;
static unsigned int columns;
static unsigned int cursor;
static unsigned int wantcolumn;
static unsigned int keepwant;
static unsigned int top;
static unsigned int left;
static unsigned int mode;
static unsigned int count;
static unsigned int operator;
static unsigned int pending;
static unsigned int redraw;
static unsigned int editline;
static unsigned int modified;
static unsigned int eol = 1;
static unsigned int escaped;
static unsigned int param;
static unsigned int lastbyte;

static void flush(void)
{

    if (outcount)
        channel_send(0, output, MESSAGE_TYPE_DATA, outcount, outdata);

    outcount = 0;

}

static void emit(void *data, unsigned int size)
{

    if (outcount + size > MESSAGE_SIZE)
        flush();

    outcount += buffer_write(outdata, MESSAGE_SIZE, data, size, outcount);

}

static void emitbyte(unsigned char c)
{

    emit(&c, 1);

}

static void emitstring(char *string)
{

    emit(string, cstring_length(string));

}

static void emitfmt2(char *fmt, void *arg1, void *arg2)
{

    char buffer[64];

    emit(buffer, cstring_write_fmt(buffer, 64, 0, fmt, arg1, arg2));

}

static void setmessage(char *fmt, void *arg1, void *arg2, void *arg3)
{

    cstring_write_fmt(notice, MESSAGESIZE, 0, fmt, arg1, arg2, arg3);

}

static char *filename(void)
{

    return (path[0]) ? path + fs_dirlength(path) : "[No Name]";

}

static unsigned int iscontinuation(unsigned int pos)
{

    return pos < textcount && (text[pos] & 0xC0) == 0x80;

}

static unsigned int nextchar(unsigned int pos)
{

    if (pos < textcount)
        pos++;

    while (iscontinuation(pos))
        pos++;

    return pos;

}

static unsigned int prevchar(unsigned int pos)
{

    if (pos)
        pos--;

    while (pos && iscontinuation(pos))
        pos--;

    return pos;

}

static unsigned int linestart(unsigned int pos)
{

    while (pos && text[pos - 1] != '\n')
        pos--;

    return pos;

}

static unsigned int lineend(unsigned int pos)
{

    while (pos < textcount && text[pos] != '\n')
        pos++;

    return pos;

}

static unsigned int linenumber(unsigned int pos)
{

    unsigned int number = 0;
    unsigned int i;

    for (i = 0; i < pos; i++)
    {

        if (text[i] == '\n')
            number++;

    }

    return number;

}

static unsigned int lineat(unsigned int number)
{

    unsigned int pos = 0;

    while (number)
    {

        unsigned int end = lineend(pos);

        if (end == textcount)
            break;

        pos = end + 1;
        number--;

    }

    return pos;

}

static unsigned int firstnonblank(unsigned int pos)
{

    pos = linestart(pos);

    while (pos < textcount && (text[pos] == ' ' || text[pos] == '\t'))
        pos++;

    return pos;

}

static unsigned int nextcolumn(unsigned int pos, unsigned int column)
{

    if (text[pos] == '\t')
        return (column / TABSIZE + 1) * TABSIZE;

    return ((text[pos] & 0xC0) == 0x80) ? column : column + 1;

}

static unsigned int columnof(unsigned int pos)
{

    unsigned int column = 0;
    unsigned int i;

    for (i = linestart(pos); i < pos; i++)
        column = nextcolumn(i, column);

    return column;

}

static unsigned int atcolumn(unsigned int start, unsigned int column)
{

    unsigned int end = lineend(start);
    unsigned int current = 0;
    unsigned int pos = start;

    while (pos < end)
    {

        unsigned int next = nextcolumn(pos, current);

        if (next > column)
            return pos;

        current = next;
        pos = nextchar(pos);

    }

    return pos;

}

static unsigned int charclass(unsigned int pos)
{

    unsigned char c;

    if (pos >= textcount)
        return 0;

    c = text[pos];

    if (c == ' ' || c == '\t' || c == '\n')
        return 0;

    if (c >= 0x80 || c == '_' || cstring_isalpha(c) || cstring_isdigit(c))
        return 1;

    return 2;

}

static void touchline(unsigned int number)
{

    if (redraw == REDRAW_NONE)
    {

        redraw = REDRAW_LINE;
        editline = number;

    }

    else if (redraw == REDRAW_LINE && editline != number)
    {

        redraw = REDRAW_ALL;

    }

}

static unsigned int makeroom(unsigned int pos, unsigned int size)
{

    unsigned int i;

    if (size > TEXTSIZE - textcount)
    {

        setmessage("E: Text buffer is full\\0", (void *)0, (void *)0, (void *)0);

        return 0;

    }

    for (i = textcount; i > pos; i--)
        text[i + size - 1] = text[i - 1];

    textcount += size;
    modified = 1;

    return 1;

}

static unsigned int insertbyte(unsigned int pos, unsigned char c)
{

    if (!makeroom(pos, 1))
        return 0;

    text[pos] = c;

    if (c == '\n')
        redraw = REDRAW_ALL;
    else
        touchline(linenumber(pos));

    return 1;

}

static unsigned int insertyank(unsigned int pos)
{

    unsigned int i;

    if (!makeroom(pos, yankcount))
        return 0;

    for (i = 0; i < yankcount; i++)
        text[pos + i] = yank[i];

    redraw = REDRAW_ALL;

    return 1;

}

static void deleterange(unsigned int from, unsigned int to)
{

    unsigned int i;

    if (from >= to)
        return;

    if (buffer_findbyte(text + from, to - from, '\n') < to - from)
        redraw = REDRAW_ALL;
    else
        touchline(linenumber(from));

    for (i = to; i < textcount; i++)
        text[i - (to - from)] = text[i];

    textcount -= to - from;
    modified = 1;

}

static void yankrange(unsigned int from, unsigned int to, unsigned int linewise)
{

    unsigned int i;

    yankcount = 0;
    yanklines = linewise;

    for (i = from; i < to; i++)
        yank[yankcount++] = text[i];

    if (linewise)
        yank[yankcount++] = '\n';

}

static void snapshot(void)
{

    buffer_copy(undotext, text, textcount);

    undocount = textcount;
    undocursor = cursor;
    hasundo = 1;

}

static void undo(void)
{

    unsigned int size = (textcount > undocount) ? textcount : undocount;
    unsigned int value;
    unsigned int i;

    if (!hasundo)
    {

        setmessage("Already at oldest change\\0", (void *)0, (void *)0, (void *)0);

        return;

    }

    for (i = 0; i < size; i++)
    {

        unsigned char c = text[i];

        text[i] = undotext[i];
        undotext[i] = c;

    }

    value = textcount;
    textcount = undocount;
    undocount = value;
    value = cursor;
    cursor = undocursor;
    undocursor = value;
    modified = 1;
    redraw = REDRAW_ALL;

}

static struct motion motion(unsigned int key, unsigned int number)
{

    unsigned int n = (number) ? number : 1;
    struct motion m;
    unsigned int i;

    m.position = cursor;
    m.linewise = 0;
    m.inclusive = 0;

    switch (key)
    {

    case 'h':
    case 0x08:
    case 0x7F:
    case KEY_LEFT:
        for (i = 0; i < n && m.position > linestart(m.position); i++)
            m.position = prevchar(m.position);

        break;

    case 'l':
    case ' ':
    case KEY_RIGHT:
        for (i = 0; i < n && m.position < lineend(m.position); i++)
            m.position = nextchar(m.position);

        break;

    case 'j':
    case 0x0E:
    case KEY_DOWN:
        for (i = 0; i < n && lineend(m.position) < textcount; i++)
            m.position = lineend(m.position) + 1;

        m.position = atcolumn(linestart(m.position), wantcolumn);
        m.linewise = 1;
        keepwant = 1;

        break;

    case 'k':
    case 0x10:
    case KEY_UP:
        for (i = 0; i < n && linestart(m.position); i++)
            m.position = linestart(linestart(m.position) - 1);

        m.position = atcolumn(linestart(m.position), wantcolumn);
        m.linewise = 1;
        keepwant = 1;

        break;

    case '+':
    case 0x0D:
        for (i = 0; i < n && lineend(m.position) < textcount; i++)
            m.position = lineend(m.position) + 1;

        m.position = firstnonblank(m.position);
        m.linewise = 1;

        break;

    case '-':
        for (i = 0; i < n && linestart(m.position); i++)
            m.position = linestart(linestart(m.position) - 1);

        m.position = firstnonblank(m.position);
        m.linewise = 1;

        break;

    case '0':
    case KEY_HOME:
        m.position = linestart(m.position);

        break;

    case '^':
        m.position = firstnonblank(m.position);

        break;

    case '$':
    case KEY_END:
        for (i = 1; i < n && lineend(m.position) < textcount; i++)
            m.position = lineend(m.position) + 1;

        m.position = lineend(m.position);
        wantcolumn = COLUMNEND;
        keepwant = 1;

        break;

    case 'w':
        for (i = 0; i < n && m.position < textcount; i++)
        {

            unsigned int c = charclass(m.position);

            while (c && charclass(m.position) == c)
                m.position++;

            while (m.position < textcount && !charclass(m.position))
            {

                if (text[m.position] == '\n' && m.position + 1 < textcount && text[m.position + 1] == '\n')
                {

                    m.position++;

                    break;

                }

                m.position++;

            }

        }

        break;

    case 'b':
        for (i = 0; i < n && m.position; i++)
        {

            unsigned int c;

            m.position--;

            while (m.position && !charclass(m.position))
                m.position--;

            c = charclass(m.position);

            while (m.position && charclass(m.position - 1) == c)
                m.position--;

        }

        break;

    case 'e':
        for (i = 0; i < n && m.position + 1 < textcount; i++)
        {

            unsigned int c;

            m.position++;

            while (m.position + 1 < textcount && !charclass(m.position))
                m.position++;

            c = charclass(m.position);

            while (m.position + 1 < textcount && charclass(m.position + 1) == c)
                m.position++;

        }

        while (iscontinuation(m.position))
            m.position--;

        m.inclusive = 1;

        break;

    case 'G':
        m.position = firstnonblank((number) ? lineat(number - 1) : lineat(linenumber(textcount)));
        m.linewise = 1;

        break;

    case KEY_GG:
        m.position = firstnonblank(lineat((number) ? number - 1 : 0));
        m.linewise = 1;

        break;

    default:
        m.position = NOMOTION;

        break;

    }

    return m;

}

static void applyoperator(unsigned int op, struct motion m)
{

    unsigned int from = (m.position < cursor) ? m.position : cursor;
    unsigned int to = (m.position < cursor) ? cursor : m.position;

    if (m.linewise)
    {

        from = linestart(from);
        to = lineend(to);

        yankrange(from, to, 1);

        if (op == 'y')
        {

            cursor = (m.position < cursor) ? m.position : cursor;

            return;

        }

        snapshot();

        if (op == 'c')
        {

            deleterange(from, to);

            cursor = from;
            mode = MODE_INSERT;

            return;

        }

        if (to < textcount)
            to++;
        else if (from)
            from--;

        deleterange(from, to);

        cursor = firstnonblank(from);

        return;

    }

    if (m.inclusive)
        to = nextchar(to);

    if (from == to)
    {

        if (op == 'c')
        {

            snapshot();

            mode = MODE_INSERT;

        }

        return;

    }

    yankrange(from, to, 0);

    cursor = from;

    if (op == 'y')
        return;

    snapshot();
    deleterange(from, to);

    if (op == 'c')
        mode = MODE_INSERT;

}

static void runoperator(unsigned int op, unsigned int key, unsigned int number)
{

    struct motion m;

    if (op == 'c' && key == 'w' && charclass(cursor))
        key = 'e';

    m = motion(key, number);

    if (m.position == NOMOTION)
        return;

    if (key == 'w' && m.position > lineend(cursor))
        m.position = lineend(cursor);

    applyoperator(op, m);

}

static void runlines(unsigned int op, unsigned int n)
{

    struct motion m;

    m.position = lineat(linenumber(cursor) + n - 1);
    m.linewise = 1;
    m.inclusive = 0;

    applyoperator(op, m);

}

static void paste(unsigned int n, unsigned int after)
{

    unsigned int pos;
    unsigned int i;

    if (!yankcount)
        return;

    snapshot();

    if (yanklines)
    {

        unsigned int atend = 0;

        pos = (after) ? lineend(cursor) : linestart(cursor);

        if (after && pos == textcount)
        {

            atend = insertbyte(pos, '\n');
            pos = textcount;

        }

        else if (after)
        {

            pos++;

        }

        for (i = 0; i < n; i++)
            insertyank(pos);

        if (atend)
            deleterange(textcount - 1, textcount);

        cursor = firstnonblank(pos);

    }

    else
    {

        pos = (after && cursor < lineend(cursor)) ? nextchar(cursor) : cursor;

        for (i = 0; i < n; i++)
            insertyank(pos);

        cursor = prevchar(pos + yankcount * n);

    }

}

static void join(unsigned int n)
{

    unsigned int times = (n > 1) ? n - 1 : 1;
    unsigned int i;

    snapshot();

    for (i = 0; i < times; i++)
    {

        unsigned int end = lineend(cursor);
        unsigned int next = end + 1;

        if (end == textcount)
            break;

        while (next < textcount && (text[next] == ' ' || text[next] == '\t'))
            next++;

        deleterange(end, next);

        if (end > linestart(end) && end < textcount && text[end] != '\n' && text[end - 1] != ' ')
            insertbyte(end, ' ');

        cursor = end;

    }

}

static void replacechars(unsigned int key, unsigned int n)
{

    unsigned int pos = cursor;
    unsigned int i;

    if (key < 0x20 || key > 0x7E)
        return;

    for (i = 0; i < n; i++)
    {

        if (pos >= lineend(cursor))
            return;

        pos = nextchar(pos);

    }

    snapshot();

    for (i = 0; i < n; i++)
    {

        deleterange(cursor, nextchar(cursor));
        insertbyte(cursor, key);

        cursor++;

    }

    cursor--;

}

static void togglecase(unsigned int n)
{

    unsigned int i;

    snapshot();

    for (i = 0; i < n && cursor < lineend(cursor); i++)
    {

        if (text[cursor] >= 'a' && text[cursor] <= 'z')
            text[cursor] -= 32;
        else if (text[cursor] >= 'A' && text[cursor] <= 'Z')
            text[cursor] += 32;

        modified = 1;

        touchline(linenumber(cursor));

        cursor = nextchar(cursor);

    }

}

static unsigned int matchat(unsigned int pos)
{

    unsigned int i;

    for (i = 0; i < searchcount; i++)
    {

        if (pos + i >= textcount || text[pos + i] != (unsigned char)search[i])
            return 0;

    }

    return 1;

}

static void find(unsigned int forward)
{

    unsigned int pos = cursor;
    unsigned int i;

    if (!searchcount)
    {

        setmessage("E35: No previous regular expression\\0", (void *)0, (void *)0, (void *)0);

        return;

    }

    for (i = 0; i < textcount; i++)
    {

        if (forward)
            pos = (pos + 1 < textcount) ? pos + 1 : 0;
        else
            pos = (pos) ? pos - 1 : textcount - 1;

        if (matchat(pos))
        {

            if (forward && pos <= cursor)
                setmessage("search hit BOTTOM, continuing at TOP\\0", (void *)0, (void *)0, (void *)0);
            else if (!forward && pos >= cursor)
                setmessage("search hit TOP, continuing at BOTTOM\\0", (void *)0, (void *)0, (void *)0);

            cursor = pos;

            return;

        }

    }

    setmessage("E486: Pattern not found: %s\\0", search, (void *)0, (void *)0);

}

static void scroll(unsigned int down, unsigned int amount, unsigned int movecursor)
{

    unsigned int total = linenumber(textcount) + 1;
    unsigned int number = linenumber(cursor);

    if (down)
    {

        top = (top + amount < total) ? top + amount : total - 1;

        if (movecursor)
            number = (number + amount < total) ? number + amount : total - 1;

        if (number < top)
            number = top;

    }

    else
    {

        top = (top > amount) ? top - amount : 0;

        if (movecursor)
            number = (number > amount) ? number - amount : 0;

        if (number >= top + rows - 1)
            number = top + rows - 2;

    }

    cursor = firstnonblank(lineat(number));
    redraw = REDRAW_ALL;

}

static void loadfile(void)
{

    unsigned int target = fs_auth(path);
    unsigned int id = (target) ? fs_walk(1, target, 0, path) : 0;
    struct record record;
    unsigned int lines;

    textcount = 0;
    cursor = 0;
    top = 0;
    left = 0;
    eol = 1;
    modified = 0;
    hasundo = 0;

    if (!id)
    {

        setmessage("\"%s\" [New File]\\0", filename(), (void *)0, (void *)0);

        return;

    }

    if (!fs_stat(1, target, id, &record) || record.type == RECORD_TYPE_DIRECTORY || record.size > TEXTSIZE)
    {

        setmessage("\"%s\" is a directory or too large\\0", filename(), (void *)0, (void *)0);

        path[0] = '\0';

        return;

    }

    textcount = fs_read_full(1, target, id, text, record.size, 0);

    if (textcount && text[textcount - 1] == '\n')
        textcount--;
    else if (textcount)
        eol = 0;

    lines = (textcount || eol) ? linenumber(textcount) + 1 : 0;

    setmessage("\"%s\" %uL, %uB\\0", filename(), &lines, &record.size);

}

static unsigned int writefile(char *name)
{

    unsigned int target = fs_auth(name);
    unsigned int parent = (target) ? fs_walkparent(1, target, name) : 0;
    char *base = name + fs_dirlength(name);
    unsigned int newline = eol && textcount;
    unsigned int lines = (textcount) ? linenumber(textcount) + 1 : 0;
    unsigned int size = textcount + newline;
    unsigned int id;

    if (!parent || !cstring_length(base))
    {

        setmessage("E212: Can't open file for writing: %s\\0", name, (void *)0, (void *)0);

        return 0;

    }

    id = fs_walk(1, target, parent, base);

    if (id && !fs_remove(1, target, parent, id))
    {

        setmessage("E212: Can't replace file: %s\\0", name, (void *)0, (void *)0);

        return 0;

    }

    id = fs_create(1, target, parent, base, cstring_length(base));

    if (!id || fs_write_all(1, target, id, text, textcount, 0) != textcount || (newline && fs_write_all(1, target, id, "\n", 1, textcount) != 1))
    {

        setmessage("E212: Can't write file: %s\\0", name, (void *)0, (void *)0);

        return 0;

    }

    setmessage("\"%s\" %uL, %uB written\\0", base, &lines, &size);

    return 1;

}

static unsigned int save(char *argument)
{

    if (cstring_length(argument))
    {

        char absolute[PATHSIZE];

        fs_absolute(absolute, PATHSIZE, option_getstring("pwd"), argument);

        if (path[0] && !cstring_match(absolute, path))
            return writefile(absolute);

        cstring_write_fmt(path, PATHSIZE, 0, "%s\\0", absolute);

    }

    if (!path[0])
    {

        setmessage("E32: No file name\\0", (void *)0, (void *)0, (void *)0);

        return 0;

    }

    if (!writefile(path))
        return 0;

    modified = 0;

    return 1;

}

static void quit(unsigned int force)
{

    if (modified && !force)
        setmessage("E37: No write since last change (add ! to override)\\0", (void *)0, (void *)0, (void *)0);
    else
        channel_close(0);

}

static void gotoline(unsigned int number)
{

    cursor = firstnonblank(lineat((number) ? number - 1 : 0));

}

static unsigned int iscommand(unsigned int start, unsigned int length, char *name)
{

    return length == cstring_length(name) && buffer_match(line + start, name, length);

}

static void execute(void)
{

    unsigned int i = 0;
    unsigned int start;
    unsigned int length;
    unsigned int force = 0;
    char *argument;

    line[linecount] = '\0';

    while (i < linecount && line[i] == ' ')
        i++;

    if (i == linecount)
        return;

    if (line[i] == '$')
    {

        gotoline(linenumber(textcount) + 1);

        return;

    }

    if (cstring_isdigit(line[i]))
    {

        unsigned int number = 0;

        while (i < linecount && cstring_isdigit(line[i]))
            number = number * 10 + cstring_toint(line[i++]);

        gotoline(number);

        return;

    }

    start = i;

    while (i < linecount && cstring_isalpha(line[i]))
        i++;

    length = i - start;

    if (i < linecount && line[i] == '!')
    {

        force = 1;
        i++;

    }

    while (i < linecount && line[i] == ' ')
        i++;

    argument = line + i;

    if (iscommand(start, length, "w"))
        save(argument);
    else if (iscommand(start, length, "q"))
        quit(force);
    else if (iscommand(start, length, "wq"))
    {

        if (save(argument))
            quit(1);

    }

    else if (iscommand(start, length, "x"))
    {

        if (!modified || save(argument))
            quit(1);

    }

    else
        setmessage("E492: Not an editor command: %s\\0", line, (void *)0, (void *)0);

}

static void showinfo(void)
{

    unsigned int lines = linenumber(textcount) + 1;

    setmessage("\"%s\"%s %u lines\\0", filename(), (modified) ? " [Modified]" : "", &lines);

}

static void startinsert(void)
{

    snapshot();

    mode = MODE_INSERT;

}

static void startline(unsigned int key)
{

    mode = (key == ':') ? MODE_COMMAND : MODE_SEARCH;
    searchforward = (key == '/');
    linecount = 0;

}

static void command(unsigned int key, unsigned int n)
{

    unsigned int pos;

    switch (key)
    {

    case 'i':
        startinsert();

        break;

    case 'a':
        startinsert();

        if (cursor < lineend(cursor))
            cursor = nextchar(cursor);

        break;

    case 'I':
        startinsert();

        cursor = firstnonblank(cursor);

        break;

    case 'A':
        startinsert();

        cursor = lineend(cursor);

        break;

    case 'o':
        startinsert();

        pos = lineend(cursor);

        if (insertbyte(pos, '\n'))
            cursor = pos + 1;

        break;

    case 'O':
        startinsert();

        pos = linestart(cursor);

        if (insertbyte(pos, '\n'))
            cursor = pos;

        break;

    case 'x':
    case KEY_DELETE:
        runoperator('d', 'l', n);

        break;

    case 'X':
        runoperator('d', 'h', n);

        break;

    case 'D':
        runoperator('d', '$', n);

        break;

    case 'C':
        runoperator('c', '$', n);

        break;

    case 's':
        runoperator('c', 'l', n);

        break;

    case 'S':
        runlines('c', n);

        break;

    case 'Y':
        runlines('y', n);

        break;

    case 'p':
        paste(n, 1);

        break;

    case 'P':
        paste(n, 0);

        break;

    case 'J':
        join(n);

        break;

    case 'u':
        undo();

        break;

    case '~':
        togglecase(n);

        break;

    case 'n':
        find(searchforward);

        break;

    case 'N':
        find(!searchforward);

        break;

    case ':':
    case '/':
    case '?':
        startline(key);

        break;

    case 0x02:
    case KEY_PAGEUP:
        scroll(0, (rows > 3) ? rows - 3 : 1, 0);

        break;

    case 0x06:
    case KEY_PAGEDOWN:
        scroll(1, (rows > 3) ? rows - 3 : 1, 0);

        break;

    case 0x04:
        scroll(1, rows / 2, 1);

        break;

    case 0x15:
        scroll(0, rows / 2, 1);

        break;

    case 0x07:
        showinfo();

        break;

    case 0x0C:
        redraw = REDRAW_ALL;

        break;

    }

}

static void normalkey(unsigned int key)
{

    unsigned int n = (count) ? count : 1;
    struct motion m;

    if (pending)
    {

        unsigned int prefix = pending;

        pending = 0;

        switch (prefix)
        {

        case 'r':
            replacechars(key, n);

            break;

        case 'Z':
            if (key == 'Z' && (!modified || save("")))
                quit(1);
            else if (key == 'Q')
                quit(1);

            break;

        case 'g':
            if (key == 'g')
                key = KEY_GG;
            else
                key = KEY_NONE;

            break;

        }

        if (key != KEY_GG)
        {

            count = 0;
            operator = 0;

            return;

        }

    }

    if (key < 0x80 && cstring_isdigit(key) && (key != '0' || count))
    {

        count = count * 10 + cstring_toint(key);
        keepwant = 1;

        return;

    }

    if (key == 'd' || key == 'c' || key == 'y')
    {

        if (!operator)
        {

            operator = key;
            keepwant = 1;

            return;

        }

        if (operator == key)
            runlines(key, n);

        count = 0;
        operator = 0;

        return;

    }

    if (key == 'g' || ((key == 'r' || key == 'Z') && !operator))
    {

        pending = key;
        keepwant = 1;

        return;

    }

    if (operator)
    {

        runoperator(operator, key, count);

    }

    else
    {

        m = motion(key, count);

        if (m.position != NOMOTION)
            cursor = m.position;
        else
            command(key, n);

    }

    count = 0;
    operator = 0;

}

static void insertkey(unsigned int key)
{

    struct motion m;
    unsigned int pos;

    switch (key)
    {

    case 0x1B:
        mode = MODE_NORMAL;

        if (cursor > linestart(cursor))
            cursor = prevchar(cursor);

        break;

    case 0x08:
    case 0x7F:
        pos = prevchar(cursor);

        deleterange(pos, cursor);

        cursor = pos;

        break;

    case KEY_DELETE:
        deleterange(cursor, nextchar(cursor));

        break;

    case 0x0A:
    case 0x0D:
        if (insertbyte(cursor, '\n'))
            cursor++;

        break;

    case KEY_UP:
    case KEY_DOWN:
    case KEY_LEFT:
    case KEY_RIGHT:
    case KEY_HOME:
    case KEY_END:
        m = motion(key, 1);
        cursor = m.position;

        break;

    default:
        if ((key == 0x09 || key >= 0x20) && key < 0x100 && key != 0x7F && insertbyte(cursor, key))
            cursor++;

        break;

    }

}

static void linekey(unsigned int key)
{

    keepwant = 1;

    switch (key)
    {

    case 0x1B:
        mode = MODE_NORMAL;

        break;

    case 0x08:
    case 0x7F:
        if (!linecount)
        {

            mode = MODE_NORMAL;

            break;

        }

        do
        {

            linecount--;

        } while (linecount && (line[linecount] & 0xC0) == 0x80);

        break;

    case 0x0A:
    case 0x0D:
        keepwant = 0;

        if (mode == MODE_SEARCH)
        {

            mode = MODE_NORMAL;

            if (linecount)
            {

                searchcount = buffer_write(search, LINESIZE - 1, line, linecount, 0);
                search[searchcount] = '\0';

            }

            find(searchforward);

        }

        else
        {

            mode = MODE_NORMAL;

            execute();

        }

        break;

    default:
        if (key >= 0x20 && key < 0x100 && linecount < LINESIZE - 1)
            line[linecount++] = key;

        break;

    }

}

static void onkey(unsigned int key)
{

    keepwant = 0;

    switch (mode)
    {

    case MODE_INSERT:
        notice[0] = '\0';

        insertkey(key);

        break;

    case MODE_COMMAND:
    case MODE_SEARCH:
        notice[0] = '\0';

        linekey(key);

        break;

    default:
        normalkey(key);

        break;

    }

    if (cursor > textcount)
        cursor = textcount;

    if (mode == MODE_NORMAL && cursor == lineend(cursor) && cursor > linestart(cursor))
        cursor = prevchar(cursor);

    if (!keepwant)
        wantcolumn = columnof(cursor);

}

static unsigned int translate(unsigned char c)
{

    switch (c)
    {

    case 'A':
        return KEY_UP;

    case 'B':
        return KEY_DOWN;

    case 'C':
        return KEY_RIGHT;

    case 'D':
        return KEY_LEFT;

    case 'H':
        return KEY_HOME;

    case 'F':
        return KEY_END;

    case '~':
        switch (param)
        {

        case 1:
        case 7:
            return KEY_HOME;

        case 3:
            return KEY_DELETE;

        case 4:
        case 8:
            return KEY_END;

        case 5:
            return KEY_PAGEUP;

        case 6:
            return KEY_PAGEDOWN;

        }

        break;

    }

    return KEY_NONE;

}

static void drawline(unsigned int row, unsigned int start)
{

    unsigned int column = 0;
    unsigned int visible = 0;
    unsigned int pos;

    emitfmt2("\033[%u;1H", &row, (void *)0);

    for (pos = start; pos < textcount && text[pos] != '\n' && column < left + columns; pos++)
    {

        unsigned char c = text[pos];

        if (c == '\t')
        {

            unsigned int next = nextcolumn(pos, column);

            for (; column < next && column < left + columns; column++)
            {

                if (column >= left)
                    emitbyte(' ');

            }

        }

        else if ((c & 0xC0) == 0x80)
        {

            if (visible)
                emitbyte(c);

        }

        else
        {

            visible = column >= left;

            if (visible)
                emitbyte((c < 0x20 || c == 0x7F) ? '?' : c);

            column++;

        }

    }

    emitstring("\033[K");

}

static void drawall(void)
{

    unsigned int pos = lineat(top);
    unsigned int more = (linenumber(textcount) >= top);
    unsigned int row;

    for (row = 1; row < rows; row++)
    {

        if (more)
        {

            drawline(row, pos);

            pos = lineend(pos);
            more = pos < textcount;
            pos++;

        }

        else
        {

            emitfmt2("\033[%u;1H~\033[K", &row, (void *)0);

        }

    }

}

static void drawstatus(unsigned int number, unsigned int column)
{

    char status[STATUSSIZE];
    unsigned int ruler = columns - RULERSIZE;
    unsigned int size = 0;

    if (mode == MODE_COMMAND || mode == MODE_SEARCH)
    {

        status[0] = (mode == MODE_COMMAND) ? ':' : ((searchforward) ? '/' : '?');
        size = 1 + buffer_write(status, columns - 1, line, linecount, 1);

    }

    else if (notice[0])
    {

        size = cstring_write(status, columns - 1, notice, 0);

    }

    else if (mode == MODE_INSERT)
    {

        size = cstring_write(status, columns - 1, "-- INSERT --", 0);

    }

    emitfmt2("\033[%u;1H", &rows, (void *)0);
    emit(status, size);
    emitstring("\033[K");

    if (mode != MODE_COMMAND && mode != MODE_SEARCH && size < ruler)
    {

        number++;
        column++;
        ruler++;

        emitfmt2("\033[%u;%uH", &rows, &ruler);
        emitfmt2("%u,%u", &number, &column);

    }

}

static void render(void)
{

    unsigned int number = linenumber(cursor);
    unsigned int column = columnof(cursor);
    unsigned int row;

    if (number < top)
    {

        top = number;
        redraw = REDRAW_ALL;

    }

    if (number >= top + rows - 1)
    {

        top = number - rows + 2;
        redraw = REDRAW_ALL;

    }

    if (column < left)
    {

        left = column;
        redraw = REDRAW_ALL;

    }

    if (column >= left + columns)
    {

        left = column - columns + 1;
        redraw = REDRAW_ALL;

    }

    if (redraw == REDRAW_ALL)
        drawall();
    else if (redraw == REDRAW_LINE && editline >= top && editline < top + rows - 1)
        drawline(editline - top + 1, lineat(editline));

    drawstatus(number, column);

    if (mode == MODE_COMMAND || mode == MODE_SEARCH)
    {

        column = linecount + 2;

        emitfmt2("\033[%u;%uH", &rows, &column);

    }

    else
    {

        row = number - top + 1;
        column = column - left + 1;

        emitfmt2("\033[%u;%uH", &row, &column);

    }

    redraw = REDRAW_NONE;

    flush();

}

static void onconsoledata(struct message *message)
{

    unsigned char data[MESSAGE_SIZE];
    unsigned int size = buffer_read(data, MESSAGE_SIZE, message->data, message->length, 0);
    unsigned int i;

    for (i = 0; i < size; i++)
    {

        unsigned char c = data[i];
        unsigned int previous = lastbyte;

        lastbyte = c;

        if (escaped == 1)
        {

            escaped = 0;

            if (c == '[' || c == 'O')
            {

                escaped = 2;
                param = 0;

                continue;

            }

            onkey(0x1B);

        }

        if (escaped >= 2)
        {

            if (escaped == 2 && cstring_isdigit(c))
                param = param * 10 + cstring_toint(c);
            else if (c == ';')
                escaped = 3;

            if (c >= 0x40 && c <= 0x7E)
            {

                escaped = 0;

                onkey(translate(c));

            }

            continue;

        }

        if (c == 0x1B)
            escaped = 1;
        else if (!(c == 0x0A && previous == 0x0D))
            onkey(c);

    }

    if (escaped == 1)
    {

        escaped = 0;

        onkey(0x1B);

    }

    render();

}

static void onmain(struct message *message)
{

    int r = option_getdecimal("rows");
    int c = option_getdecimal("columns");

    output = message->source;
    rows = (r > 2) ? r : 24;
    columns = (c > RULERSIZE * 2 && c < STATUSSIZE) ? c : 80;
    redraw = REDRAW_ALL;

    emitstring("\033[?1049h\033[H\033[2J");
    render();
    channel_hold(0);
    emitstring("\033[2J\033[H\033[?1049l");
    flush();

}

static void onpath(struct message *message)
{

    if (path[0] || modified)
    {

        setmessage("Only one file can be edited\\0", (void *)0, (void *)0, (void *)0);

    }

    else
    {

        cstring_write_fmt(path, PATHSIZE, 0, "%s\\0", message->data);
        loadfile();

    }

    redraw = REDRAW_ALL;

    render();

}

void init(void)
{

    option_add("rows", "24");
    option_add("columns", "80");
    channel_bind(MESSAGE_TYPE_CONSOLEDATA, onconsoledata);
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_PATH, onpath);

}

