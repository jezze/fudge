#include <fudge.h>
#include <abi.h>
#include "kv.h"

#define ROWS                            100
#define COLUMNS                         320
#define SCREENS                         2
#define PARAMS                          4
#define LINESIZE                        (COLUMNS * 4 + 64)
#define STATE_NORMAL                    0
#define STATE_ESCAPE                    1
#define STATE_CSI                       2

struct position
{

    unsigned int row;
    unsigned int column;

};

struct terminal
{

    unsigned char cells[SCREENS][ROWS][COLUMNS];
    unsigned char reversed[SCREENS][ROWS][COLUMNS];
    unsigned int dirty[ROWS];
    unsigned int rows;
    unsigned int columns;
    unsigned int screen;
    struct position cursor;
    struct position saved;
    unsigned int drawnrow;
    unsigned int wrapnext;
    unsigned int state;
    unsigned int params[PARAMS];
    unsigned int nparams;
    unsigned int private;
    unsigned int codepoint;
    unsigned int pending;
    unsigned int reports;
    unsigned int shown;
    unsigned int reverse;
    unsigned int sentcursor;
    unsigned int sentanchor;
    unsigned int sentend;
    unsigned int changed;

};

struct output
{

    char data[MESSAGE_SIZE];
    unsigned int count;

};

static struct terminal term;
static unsigned int shell;
static unsigned int wm;

static void sendconsole(void *data, unsigned int count)
{

    if (shell)
        channel_send(0, shell, MESSAGE_TYPE_CONSOLEDATA, count, data);

}

static void sendreport(struct terminal *terminal)
{

    char buffer[64];

    sendconsole(buffer, cstring_write_fmt(buffer, 64, 0, "\033[48;%u;%u;0;0t", &terminal->rows, &terminal->columns));

}

static void markall(struct terminal *terminal)
{

    unsigned int row;

    for (row = 0; row < ROWS; row++)
        terminal->dirty[row] = 1;

}

static void clearcells(struct terminal *terminal, unsigned int screen, unsigned int row, unsigned int from, unsigned int to)
{

    unsigned int column;

    for (column = from; column < to; column++)
    {

        terminal->cells[screen][row][column] = ' ';
        terminal->reversed[screen][row][column] = 0;

    }

    terminal->dirty[row] = 1;

}

static void clearrows(struct terminal *terminal, unsigned int from, unsigned int to)
{

    unsigned int row;

    for (row = from; row < to; row++)
        clearcells(terminal, terminal->screen, row, 0, COLUMNS);

}

static void scrollup(struct terminal *terminal, unsigned int count)
{

    unsigned int row;

    if (count > terminal->rows)
        count = terminal->rows;

    for (row = 0; row + count < terminal->rows; row++)
    {

        buffer_copy(terminal->cells[terminal->screen][row], terminal->cells[terminal->screen][row + count], COLUMNS);
        buffer_copy(terminal->reversed[terminal->screen][row], terminal->reversed[terminal->screen][row + count], COLUMNS);

    }

    clearrows(terminal, terminal->rows - count, terminal->rows);
    markall(terminal);

}

static void setcursor(struct terminal *terminal, unsigned int row, unsigned int column)
{

    terminal->cursor.row = (row < terminal->rows) ? row : terminal->rows - 1;
    terminal->cursor.column = (column < terminal->columns) ? column : terminal->columns - 1;
    terminal->wrapnext = 0;

}

static void linefeed(struct terminal *terminal)
{

    terminal->wrapnext = 0;

    if (terminal->cursor.row + 1 < terminal->rows)
        terminal->cursor.row++;
    else
        scrollup(terminal, 1);

}

static void put(struct terminal *terminal, unsigned char c)
{

    if (terminal->wrapnext)
    {

        terminal->cursor.column = 0;

        linefeed(terminal);

    }

    terminal->cells[terminal->screen][terminal->cursor.row][terminal->cursor.column] = c;
    terminal->reversed[terminal->screen][terminal->cursor.row][terminal->cursor.column] = terminal->reverse;
    terminal->dirty[terminal->cursor.row] = 1;

    if (terminal->cursor.column + 1 < terminal->columns)
        terminal->cursor.column++;
    else
        terminal->wrapnext = 1;

}

static unsigned char tocell(unsigned int codepoint)
{

    if (codepoint < 0x80 || (codepoint >= 0xA0 && codepoint < 0x100))
        return codepoint;

    if (codepoint == 0x20AC)
        return 0x80;

    return '?';

}

static unsigned int getparam(struct terminal *terminal, unsigned int index, unsigned int fallback)
{

    return (index < terminal->nparams && terminal->params[index]) ? terminal->params[index] : fallback;

}

static void erasedisplay(struct terminal *terminal, unsigned int mode)
{

    unsigned int row = terminal->cursor.row;
    unsigned int column = terminal->cursor.column;

    switch (mode)
    {

    case 0:
        clearcells(terminal, terminal->screen, row, column, COLUMNS);
        clearrows(terminal, row + 1, terminal->rows);

        break;

    case 1:
        clearrows(terminal, 0, row);
        clearcells(terminal, terminal->screen, row, 0, column + 1);

        break;

    case 2:
        clearrows(terminal, 0, terminal->rows);

        break;

    }

}

static void eraseline(struct terminal *terminal, unsigned int mode)
{

    unsigned int row = terminal->cursor.row;
    unsigned int column = terminal->cursor.column;

    switch (mode)
    {

    case 0:
        clearcells(terminal, terminal->screen, row, column, COLUMNS);

        break;

    case 1:
        clearcells(terminal, terminal->screen, row, 0, column + 1);

        break;

    case 2:
        clearcells(terminal, terminal->screen, row, 0, COLUMNS);

        break;

    }

}

static void savecursor(struct terminal *terminal)
{

    terminal->saved.row = terminal->cursor.row;
    terminal->saved.column = terminal->cursor.column;

}

static void setscreen(struct terminal *terminal, unsigned int screen)
{

    if (screen == terminal->screen)
        return;

    if (screen)
    {

        terminal->screen = screen;

        savecursor(terminal);
        clearrows(terminal, 0, terminal->rows);

    }

    else
    {

        terminal->screen = screen;

        setcursor(terminal, terminal->saved.row, terminal->saved.column);

    }

    markall(terminal);

}

static void setmode(struct terminal *terminal, unsigned int on)
{

    unsigned int i;

    if (!terminal->private)
        return;

    for (i = 0; i < terminal->nparams; i++)
    {

        switch (terminal->params[i])
        {

        case 47:
        case 1047:
        case 1049:
            setscreen(terminal, on);

            break;

        case 2048:
            terminal->reports = on;

            if (on)
                sendreport(terminal);

            break;

        }

    }

}

static void setstyle(struct terminal *terminal)
{

    unsigned int i;

    if (!terminal->nparams)
        terminal->reverse = 0;

    for (i = 0; i < terminal->nparams; i++)
    {

        if (terminal->params[i] == 0 || terminal->params[i] == 27)
            terminal->reverse = 0;
        else if (terminal->params[i] == 7)
            terminal->reverse = 1;

    }

}

static void runcsi(struct terminal *terminal, unsigned char c)
{

    unsigned int row = terminal->cursor.row;
    unsigned int column = terminal->cursor.column;
    unsigned int count = getparam(terminal, 0, 1);

    switch (c)
    {

    case 'A':
        setcursor(terminal, (row > count) ? row - count : 0, column);

        break;

    case 'B':
        setcursor(terminal, row + count, column);

        break;

    case 'C':
        setcursor(terminal, row, column + count);

        break;

    case 'D':
        setcursor(terminal, row, (column > count) ? column - count : 0);

        break;

    case 'G':
        setcursor(terminal, row, count - 1);

        break;

    case 'H':
    case 'f':
        setcursor(terminal, count - 1, getparam(terminal, 1, 1) - 1);

        break;

    case 'd':
        setcursor(terminal, count - 1, column);

        break;

    case 'J':
        erasedisplay(terminal, getparam(terminal, 0, 0));

        break;

    case 'K':
        eraseline(terminal, getparam(terminal, 0, 0));

        break;

    case 'h':
        setmode(terminal, 1);

        break;

    case 'l':
        setmode(terminal, 0);

        break;

    case 'm':
        setstyle(terminal);

        break;

    }

}

static void feedcontrol(struct terminal *terminal, unsigned char c)
{

    switch (c)
    {

    case 0x08:
        if (terminal->cursor.column)
            setcursor(terminal, terminal->cursor.row, terminal->cursor.column - 1);

        break;

    case 0x09:
        setcursor(terminal, terminal->cursor.row, (terminal->cursor.column + 8) & ~7u);

        break;

    case 0x0A:
    case 0x0B:
    case 0x0C:
        terminal->cursor.column = 0;

        linefeed(terminal);

        break;

    case 0x0D:
        setcursor(terminal, terminal->cursor.row, 0);

        break;

    case 0x1B:
        terminal->state = STATE_ESCAPE;

        break;

    }

}

static void feednormal(struct terminal *terminal, unsigned char c)
{

    if (c >= 0x80 && c < 0xC0)
    {

        if (terminal->pending)
        {

            terminal->codepoint = (terminal->codepoint << 6) | (c & 0x3F);

            if (!--terminal->pending)
                put(terminal, tocell(terminal->codepoint));

        }

        else
        {

            put(terminal, '?');

        }

        return;

    }

    if (terminal->pending)
    {

        terminal->pending = 0;

        put(terminal, '?');

    }

    if (c >= 0xC0)
    {

        terminal->pending = (c >= 0xF0) ? 3 : (c >= 0xE0) ? 2 : 1;
        terminal->codepoint = c & (0x3F >> terminal->pending);

    }

    else if (c < 0x20 || c == 0x7F)
    {

        feedcontrol(terminal, c);

    }

    else
    {

        put(terminal, c);

    }

}

static void feedescape(struct terminal *terminal, unsigned char c)
{

    terminal->state = STATE_NORMAL;

    switch (c)
    {

    case '[':
        terminal->state = STATE_CSI;
        terminal->nparams = 0;
        terminal->private = 0;

        break;

    case '7':
        savecursor(terminal);

        break;

    case '8':
        setcursor(terminal, terminal->saved.row, terminal->saved.column);

        break;

    }

}

static void feedcsi(struct terminal *terminal, unsigned char c)
{

    if (c == '?' && !terminal->nparams)
    {

        terminal->private = 1;

    }

    else if (cstring_isdigit(c))
    {

        if (!terminal->nparams)
            terminal->params[terminal->nparams++] = 0;

        terminal->params[terminal->nparams - 1] = terminal->params[terminal->nparams - 1] * 10 + cstring_toint(c);

    }

    else if (c == ';')
    {

        if (!terminal->nparams)
            terminal->params[terminal->nparams++] = 0;

        if (terminal->nparams < PARAMS)
            terminal->params[terminal->nparams++] = 0;

    }

    else if (c >= 0x40 && c <= 0x7E)
    {

        terminal->state = STATE_NORMAL;

        runcsi(terminal, c);

    }

    else if (c < 0x20)
    {

        feedcontrol(terminal, c);

    }

}

static void feed(struct terminal *terminal, void *buffer, unsigned int count)
{

    unsigned char data[MESSAGE_SIZE];
    unsigned int i;

    count = buffer_read(data, MESSAGE_SIZE, buffer, count, 0);
    terminal->changed = 1;

    for (i = 0; i < count; i++)
    {

        switch (terminal->state)
        {

        case STATE_ESCAPE:
            feedescape(terminal, data[i]);

            break;

        case STATE_CSI:
            feedcsi(terminal, data[i]);

            break;

        default:
            feednormal(terminal, data[i]);

            break;

        }

    }

}

static unsigned int resize(struct terminal *terminal, unsigned int rows, unsigned int columns)
{

    unsigned int screen;
    unsigned int row;

    rows = (rows < 1) ? 1 : (rows > ROWS) ? ROWS : rows;
    columns = (columns < 1) ? 1 : (columns > COLUMNS) ? COLUMNS : columns;

    if (rows == terminal->rows && columns == terminal->columns)
        return 0;

    if (terminal->cursor.row >= rows)
    {

        unsigned int count = terminal->cursor.row - rows + 1;

        scrollup(terminal, count);

        terminal->cursor.row -= count;

    }

    for (screen = 0; screen < SCREENS; screen++)
    {

        for (row = 0; row < ROWS; row++)
            clearcells(terminal, screen, row, (row < rows) ? columns : 0, COLUMNS);

    }

    terminal->rows = rows;
    terminal->columns = columns;

    setcursor(terminal, terminal->cursor.row, terminal->cursor.column);

    if (terminal->saved.row >= rows)
        terminal->saved.row = rows - 1;

    if (terminal->saved.column >= columns)
        terminal->saved.column = columns - 1;

    markall(terminal);

    terminal->changed = 1;

    return 1;

}

static unsigned int cellsize(unsigned char c)
{

    return (c < 0x80) ? 1 : (c == 0x80) ? 3 : 2;

}

static unsigned int rowend(struct terminal *terminal, unsigned int row)
{

    unsigned int end = terminal->columns;

    while (end && terminal->cells[terminal->screen][row][end - 1] == ' ' && !terminal->reversed[terminal->screen][row][end - 1])
        end--;

    if (row == terminal->cursor.row && end <= terminal->cursor.column)
        end = terminal->cursor.column + 1;

    return (end) ? end : 1;

}

static unsigned int rowsize(struct terminal *terminal, unsigned int row, unsigned int end)
{

    unsigned int size = 0;
    unsigned int column;

    for (column = 0; column < end; column++)
        size += cellsize(terminal->cells[terminal->screen][row][column]);

    return size;

}

static unsigned int writerow(struct terminal *terminal, char *line, unsigned int row, unsigned int end)
{

    unsigned int offset;
    unsigned int column;

    if (row < terminal->shown)
        offset = cstring_write_fmt(line, LINESIZE, 0, "= r%u label \"", &row);
    else
        offset = cstring_write_fmt(line, LINESIZE, 0, "+ text id \"r%u\" in \"screen\" wrap \"none\" label \"", &row);

    for (column = 0; column < end; column++)
    {

        unsigned char c = terminal->cells[terminal->screen][row][column];

        if (c == '"')
            offset += buffer_write(line, LINESIZE, "\"\\\"\"", 4, offset);
        else if (c < 0x80)
            offset += buffer_write(line, LINESIZE, &c, 1, offset);
        else
            offset += utf8_write(line + offset, LINESIZE - offset, (c == 0x80) ? 0x20AC : c);

    }

    return offset + buffer_write(line, LINESIZE, "\"\n", 2, offset);

}

static void flushoutput(struct output *output)
{

    if (output->count && wm)
        channel_send(0, wm, MESSAGE_TYPE_DATA, output->count, output->data);

    output->count = 0;

}

static void addoutput(struct output *output, char *line, unsigned int count)
{

    if (output->count + count > MESSAGE_SIZE)
        flushoutput(output);

    output->count += buffer_write(output->data, MESSAGE_SIZE, line, count, output->count);

}

static void sendcursor(unsigned int position)
{

    struct {struct message_wmcursor wmcursor; char id[8];} message;

    message.wmcursor.position = position;

    buffer_write(message.id, 8, "screen", 7, 0);

    if (wm)
        channel_send(0, wm, MESSAGE_TYPE_WMCURSOR, sizeof (struct message_wmcursor) + 7, &message);

}

static void sendselect(unsigned int anchor, unsigned int end)
{

    struct {struct message_wmselect wmselect; char id[8];} message;

    message.wmselect.anchor = anchor;
    message.wmselect.end = end;

    buffer_write(message.id, 8, "screen", 7, 0);

    if (wm)
        channel_send(0, wm, MESSAGE_TYPE_WMSELECT, sizeof (struct message_wmselect) + 7, &message);

}

static void render(struct terminal *terminal)
{

    struct output output;
    char line[LINESIZE];
    unsigned int total = 0;
    unsigned int cursor = 0;
    unsigned int anchor = 0;
    unsigned int end = 0;
    unsigned int row;

    output.count = 0;

    for (row = terminal->rows; row < terminal->shown; row++)
        addoutput(&output, line, cstring_write_fmt(line, LINESIZE, 0, "- r%u\n", &row));

    if (terminal->shown > terminal->rows)
        terminal->shown = terminal->rows;

    terminal->dirty[terminal->drawnrow] = 1;
    terminal->dirty[terminal->cursor.row] = 1;
    terminal->drawnrow = terminal->cursor.row;

    for (row = 0; row < terminal->rows; row++)
    {

        unsigned int stop = rowend(terminal, row);
        unsigned int column;

        if (row == terminal->cursor.row)
            cursor = total + rowsize(terminal, row, terminal->cursor.column);

        if (terminal->dirty[row] || row >= terminal->shown)
            addoutput(&output, line, writerow(terminal, line, row, stop));

        for (column = 0; column < stop; column++)
        {

            unsigned int size = cellsize(terminal->cells[terminal->screen][row][column]);

            if (terminal->reversed[terminal->screen][row][column])
            {

                if (end <= anchor)
                    anchor = total;

                end = total + size;

            }

            total += size;

        }

        terminal->dirty[row] = 0;

    }

    terminal->shown = terminal->rows;

    if (cursor < terminal->sentcursor && terminal->sentcursor != 0xFFFFFFFF)
        sendcursor(cursor);

    flushoutput(&output);

    if (cursor > terminal->sentcursor || terminal->sentcursor == 0xFFFFFFFF)
        sendcursor(cursor);

    if (anchor != terminal->sentanchor || end != terminal->sentend)
        sendselect(anchor, end);

    terminal->sentcursor = cursor;
    terminal->sentanchor = anchor;
    terminal->sentend = end;
    terminal->changed = 0;

}

static void setup(struct terminal *terminal)
{

    unsigned int screen;
    unsigned int row;

    for (screen = 0; screen < SCREENS; screen++)
    {

        for (row = 0; row < ROWS; row++)
            clearcells(terminal, screen, row, 0, COLUMNS);

    }

    terminal->rows = 24;
    terminal->columns = 80;
    terminal->sentcursor = 0xFFFFFFFF;

}

static void ondata(struct message *message)
{

    if (message->source == shell)
    {

        feed(&term, message->data, message->length);

    }

}

static void onerror(struct message *message)
{

    feed(&term, "[ERROR] ", 8);
    feed(&term, message->data, message->length);

}

static void onstatus(struct message *message)
{

    if (message->source == shell)
    {

        shell = 0;

        channel_close(0);

    }

}

static void onmain(struct message *message)
{

    wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        char *alfi = "initrd:data/alfi/wshell.alfi";

        channel_send(0, wm, MESSAGE_TYPE_WMRENDERFILE, cstring_length_zero(alfi), alfi);

        shell = fs_spawn(1, "initrd:bin/shell");

        if (shell)
        {

            struct message_pipe pipe;

            channel_send_fmt(0, shell, MESSAGE_TYPE_OPTION, "pwd\\0%s\\0console-service\\0\\0keyboard-service\\0\\0", option_getstring("pwd"));
            message_initpipe(&pipe);
            channel_send(0, shell, MESSAGE_TYPE_MAIN, sizeof (struct message_pipe), &pipe);

        }

        else
        {

            feed(&term, "Could not start shell\n", 22);

        }

        render(&term);

        while (channel_process(0) && channel_drain(0))
        {

            if (term.changed)
                render(&term);

        }

        if (shell)
        {

            sendconsole("\003", 1);
            channel_send(0, shell, MESSAGE_TYPE_TERM, 0, 0);

        }

        channel_send(0, wm, MESSAGE_TYPE_WMUNMAP, 0, 0);

    }

}

static void onwmevent(struct message *message)
{

    struct message_wmevent *event = message_validate(MESSAGE_TYPE_WMEVENT, message->data, message->length);

    if (event && kv_match(event, "q=resize"))
    {

        if (resize(&term, kv_getvalue(event, "rows=", 10), kv_getvalue(event, "columns=", 10)) && term.reports)
            sendreport(&term);

    }

}

static void onwmkeypress(struct message *message)
{

    struct message_wmkeypress *wmkeypress = message_validate(MESSAGE_TYPE_WMKEYPRESS, message->data, message->length);

    if (wmkeypress)
    {

        unsigned char code[4];

        buffer_copy(code, &wmkeypress->unicode, 4);

        if ((wmkeypress->keymod & KEYS_MOD_CTRL) && wmkeypress->length == 1 && code[0] >= 0x40 && code[0] < 0x80)
        {

            code[0] &= 0x1F;

            sendconsole(code, 1);

            return;

        }

        switch (wmkeypress->id)
        {

        case KEYS_KEY_ENTER:
            sendconsole("\r", 1);

            break;

        case KEYS_KEY_BACKSPACE:
            sendconsole("\177", 1);

            break;

        case KEYS_KEY_TAB:
            sendconsole("\t", 1);

            break;

        case KEYS_KEY_ESCAPE:
            sendconsole("\033", 1);

            break;

        case KEYS_KEY_CURSORUP:
            sendconsole("\033[A", 3);

            break;

        case KEYS_KEY_CURSORDOWN:
            sendconsole("\033[B", 3);

            break;

        case KEYS_KEY_CURSORRIGHT:
            sendconsole("\033[C", 3);

            break;

        case KEYS_KEY_CURSORLEFT:
            sendconsole("\033[D", 3);

            break;

        case KEYS_KEY_HOME:
            sendconsole("\033[H", 3);

            break;

        case KEYS_KEY_END:
            sendconsole("\033[F", 3);

            break;

        case KEYS_KEY_INSERT:
            sendconsole("\033[2~", 4);

            break;

        case KEYS_KEY_DELETE:
            sendconsole("\033[3~", 4);

            break;

        case KEYS_KEY_PAGEUP:
            sendconsole("\033[5~", 4);

            break;

        case KEYS_KEY_PAGEDOWN:
            sendconsole("\033[6~", 4);

            break;

        default:
            if (wmkeypress->length)
                sendconsole(code, wmkeypress->length);

            break;

        }

    }

}

void init(void)
{

    setup(&term);
    option_add("wm-service", "wm");
    channel_bind(MESSAGE_TYPE_DATA, ondata);
    channel_bind(MESSAGE_TYPE_ERROR, onerror);
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_STATUS, onstatus);
    channel_bind(MESSAGE_TYPE_WMEVENT, onwmevent);
    channel_bind(MESSAGE_TYPE_WMKEYPRESS, onwmkeypress);

}

