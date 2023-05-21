#include "cstring.h"
#include "url.h"

struct pos
{

    char *buffer;
    unsigned int buffersize;
    unsigned int bufferused;

};

static char *createstring(struct pos *pos, const char *s, const char *e, unsigned int zero)
{

    unsigned int len = e - s;
    unsigned int avail;

    if (!zero && !len)
        return 0;

    avail = pos->buffersize - pos->bufferused;

    if (len < avail)
    {

        char *out = pos->buffer + pos->bufferused;
        unsigned int i;

        for (i = 0; i < len; i++)
            out[i] = s[i];

        out[len] = '\0';
        pos->bufferused += len + 1;

        return out;

    }

    return 0;

}

static unsigned int copyout(char *buffer, unsigned int buffersize, char *s, unsigned int len, unsigned int offset)
{

    unsigned int avail = buffersize - offset;

    if (len < avail)
    {

        char *out = buffer + offset;
        unsigned int i;

        for (i = 0; i < len; i++)
            out[i] = s[i];

        out[len] = '\0';

        return len;

    }

    return 0;

}

unsigned int url_parse(struct url *url, void *buffer, unsigned int buffersize, char *s, unsigned int state)
{

    const char *start = s;
    unsigned int count = cstring_length_zero(s);
    struct pos pos;
    unsigned int i;

    pos.buffer = buffer;
    pos.buffersize = buffersize;
    pos.bufferused = 0;

    url->full = 0;
    url->authority = 0;
    url->scheme = 0;
    url->host = 0;
    url->port = 0;
    url->path = 0;
    url->query = 0;
    url->fragment = 0;
    url->userinfo = 0;

    for (i = 0; i < count; i++)
    {

        const char *end = s + i;
        const char *next = end + 1;
        char c = s[i];

        switch (state)
        {

        case URL_SCHEME:
            url->full = 1;

            switch (c)
            {

            case '\0':
                break;

            case ':':
                url->scheme = createstring(&pos, start, end, 0);
                state = URL_LIMBO1;
                start = next;

                break;

            case '+':
            case '-':
            case '.':
                break;

            default:
                if (!cstring_isalpha(c))
                    return 0;

                break;

            }

            break;

        case URL_LIMBO1:
            switch (c)
            {

            case '/':
                state = URL_LIMBO2;

                break;

            default:
                state = URL_PATH;
                i--;

                break;

            }

            break;

        case URL_LIMBO2:
            switch (c)
            {

            case '/':
                state = URL_AUTHORITY;
                start = next;

                break;

            default:
                state = URL_PATH;
                i--;

                break;

            }

            break;

        case URL_AUTHORITY:
            url->authority = 1;

            switch (c)
            {

            case '@':
                state = URL_USERINFO;
                i--;

                break;

            case '\0':
            case '/':
            case ':':
                state = URL_HOST;
                i--;

                break;

            default:
                break;

            }

            break;

        case URL_USERINFO:
            url->authority = 1;

            switch (c)
            {

            case '\0':
                url->userinfo = createstring(&pos, start, end, 0);

                break;

            case '@':
                url->userinfo = createstring(&pos, start, end, 0);
                state = URL_HOST;
                start = next;

                break;

            default:
                break;

            }

            break;

        case URL_HOST:
            url->authority = 1;

            switch (c)
            {

            case '\0':
                url->host = createstring(&pos, start, end, 0);

                break;

            case ':':
                url->host = createstring(&pos, start, end, 0);
                state = URL_PORT;
                start = next;

                break;

            case '/':
                url->host = createstring(&pos, start, end, 0);
                state = URL_PATH;
                start = next;

                break;

            default:
                break;

            }

            break;

        case URL_PORT:
            url->authority = 1;

            switch (c)
            {

            case '\0':
                url->port = createstring(&pos, start, end, 0);

                break;

            case '/':
                url->port = createstring(&pos, start, end, 0);
                state = URL_PATH;
                start = next;

                break;

            default:
                if (!cstring_isdigit(c))
                    return 0;

                break;

            }

            break;

        case URL_PATH:
            switch (c)
            {

            case '\0':
                url->path = createstring(&pos, start, end, 1);

                break;

            case '?':
                url->path = createstring(&pos, start, end, 1);
                state = URL_QUERY;
                start = next;

                break;

            case '#':
                url->path = createstring(&pos, start, end, 1);
                state = URL_FRAGMENT;
                start = next;

                break;

            default:
                break;

            }

            break;

        case URL_QUERY:
            switch (c)
            {

            case '\0':
                url->query = createstring(&pos, start, end, 1);

                break;

            case '#':
                url->query = createstring(&pos, start, end, 1);
                state = URL_FRAGMENT;
                start = next;

                break;

            default:
                break;

            }

            break;

        case URL_FRAGMENT:
            switch (c)
            {

            case '\0':
                url->fragment = createstring(&pos, start, end, 1);

                break;

            default:
                break;

            }

        }

    }

    if (url->full && !url->scheme)
        return 0;

    if (url->authority && !url->host)
        return 0;

    return 1;

}

unsigned int url_tostring(struct url *url, char *buffer, unsigned int buffersize)
{

    unsigned int offset = 0;

    if (url->scheme)
    {

        offset += copyout(buffer, buffersize, url->scheme, cstring_length(url->scheme), offset);
        offset += copyout(buffer, buffersize, ":", 1, offset);

    }

    if (url->authority)
    {

        offset += copyout(buffer, buffersize, "//", 2, offset);

        if (url->userinfo)
        {

            offset += copyout(buffer, buffersize, url->userinfo, cstring_length(url->userinfo), offset);
            offset += copyout(buffer, buffersize, "@", 1, offset);

        }

        if (url->host)
        {

            offset += copyout(buffer, buffersize, url->host, cstring_length(url->host), offset);

        }

        if (url->port)
        {

            offset += copyout(buffer, buffersize, ":", 1, offset);
            offset += copyout(buffer, buffersize, url->port, cstring_length(url->port), offset);

        }

    }

    if (url->path)
    {

        if (url->authority)
            offset += copyout(buffer, buffersize, "/", 1, offset);

        offset += copyout(buffer, buffersize, url->path, cstring_length(url->path), offset);

    }

    if (url->query)
    {

        offset += copyout(buffer, buffersize, "?", 1, offset);
        offset += copyout(buffer, buffersize, url->query, cstring_length(url->query), offset);

    }

    if (url->fragment)
    {

        offset += copyout(buffer, buffersize, "#", 1, offset);
        offset += copyout(buffer, buffersize, url->fragment, cstring_length(url->fragment), offset);

    }

    buffer[offset] = '\0';

    return offset;

}

