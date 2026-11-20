#include <fudge.h>
#include <abi.h>
#include "html.h"
#include "css.h"
#include "font.h"
#include "layout.h"
#include "paint.h"
#include "../wutils/kv.h"

#define URLSIZE                         512
#define SOURCESIZE                      0x40000
#define HISTORY                         32
#define IMAGES                          2
#define NAMES                           16
#define NAMESIZE                        32
#define TITLESIZE                       80
#define ERRORSIZE                       128
#define REDIRECTS                       5
#define MAXHEIGHT                       0x4000
#define MAXBYTES                        0x2000000

struct image
{

    unsigned int target;
    unsigned int parent;
    unsigned int id;
    char name[NAMESIZE];

};

struct page
{

    unsigned int count;
    unsigned int text;

};

static struct html_document document;
static struct css_sheet sheet;
static struct css_style styles[HTML_NODES];
static struct layout layout;
static struct font_face faces[FONT_FACES];
static struct image images[IMAGES];
static char source[SOURCESIZE];
static char history[HISTORY][URLSIZE];
static char current[URLSIZE];
static char fetched[URLSIZE];
static char base[URLSIZE];
static char typed[URLSIZE];
static char error[ERRORSIZE];
static unsigned int historycount;
static unsigned int historyindex;
static unsigned int nextimage;
static unsigned int wm;
static unsigned int viewwidth;
static unsigned int viewheight;
static char *fontpaths[FONT_FACES] = {
    "initrd:data/font/ter-112n.pcf",
    "initrd:data/font/ter-114n.pcf",
    "initrd:data/font/ter-116n.pcf",
    "initrd:data/font/ter-118n.pcf",
    "initrd:data/font/ter-112b.pcf",
    "initrd:data/font/ter-114b.pcf",
    "initrd:data/font/ter-116b.pcf",
    "initrd:data/font/ter-118b.pcf"
};
static char *home =
    "<html><head><title>Start</title><style>"
    "body{max-width:640px;margin:2em auto;padding:0 1em;color:#222;background:#f6f6f2}"
    "h1{color:#335;border-bottom:2px solid #99b;padding-bottom:4px}"
    "code{background:#e4e4dc;padding:0 3px}"
    "</style></head><body>"
    "<h1>Browser</h1>"
    "<p>Type an address in the bar above and press Enter. Addresses can be files, such as <code>root:data/html/index.html</code>, or pages over <code>http://</code>.</p>"
    "<ul><li><a href=\"initrd:data/help.txt\">initrd:data/help.txt</a></li><li><a href=\"root:data/html/index.html\">root:data/html/index.html</a> (needs the root disk)</li></ul>"
    "<p>HTML and CSS are shown. JavaScript, images and https are not supported yet.</p>"
    "</body></html>";

static char lower(char c)
{

    return (c >= 'A' && c <= 'Z') ? c + 32 : c;

}

static unsigned int matchprefix(char *text, unsigned int length, char *prefix)
{

    unsigned int count = cstring_length(prefix);
    unsigned int i;

    if (length < count)
        return 0;

    for (i = 0; i < count; i++)
    {

        if (lower(text[i]) != prefix[i])
            return 0;

    }

    return 1;

}

static unsigned int matchsuffix(char *text, char *suffix)
{

    unsigned int length = cstring_length(text);
    unsigned int count = cstring_length(suffix);

    return length >= count && matchprefix(text + length - count, count, suffix);

}

static void copystring(char *out, unsigned int size, char *in, unsigned int length)
{

    if (length >= size)
        length = size - 1;

    buffer_write(out, size, in, length, 0);

    out[length] = '\0';

}

static void moveback(char *data, unsigned int offset, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
        data[i] = data[offset + i];

}

static unsigned int issafe(char *url)
{

    unsigned int i;

    for (i = 0; url[i]; i++)
    {

        if (url[i] == '"' || url[i] == '\\' || (unsigned char)url[i] < 0x20)
            return 0;

    }

    return 1;

}

static unsigned int findscheme(char *url)
{

    unsigned int i;

    for (i = 0; url[i] && url[i] != '/' && url[i] != '?' && url[i] != '#'; i++)
    {

        if (url[i] == ':')
            return (url[i + 1] == '/' && url[i + 2] == '/') ? i : 0;

    }

    return 0;

}

static unsigned int findservice(char *url)
{

    unsigned int i;

    for (i = 0; url[i] && url[i] != '/'; i++)
    {

        if (url[i] == ':')
            return i + 1;

    }

    return 0;

}

static unsigned int originlength(char *url)
{

    unsigned int scheme = findscheme(url);
    unsigned int i;

    if (!scheme)
        return 0;

    for (i = scheme + 3; url[i] && url[i] != '/' && url[i] != '?' && url[i] != '#'; i++);

    return i;

}

static void normalize(char *url, unsigned int start)
{

    unsigned int read = start;
    unsigned int write = start;

    while (url[read] && url[read] != '?')
    {

        if (url[read] == '/' && url[read + 1] == '.' && (url[read + 2] == '/' || url[read + 2] == '\0' || url[read + 2] == '?'))
        {

            read += 2;

            continue;

        }

        if (url[read] == '/' && url[read + 1] == '.' && url[read + 2] == '.' && (url[read + 3] == '/' || url[read + 3] == '\0' || url[read + 3] == '?'))
        {

            while (write > start && url[write - 1] != '/')
                write--;

            if (write > start)
                write--;

            while (write > start && url[write - 1] != '/')
                write--;

            if (write > start)
                write--;

            read += 3;

            continue;

        }

        url[write++] = url[read++];

    }

    while (url[read])
        url[write++] = url[read++];

    url[write] = '\0';

}

static unsigned int resolve(char *out, unsigned int size, char *from, char *href)
{

    unsigned int length = cstring_length(href);
    unsigned int origin;
    unsigned int fragment;
    char link[URLSIZE];

    while (length && (href[0] == ' ' || href[0] == '\n' || href[0] == '\t'))
    {

        href++;
        length--;

    }

    fragment = buffer_findbyte(href, length, '#');

    if (!fragment || matchprefix(href, length, "javascript:") || matchprefix(href, length, "mailto:"))
        return 0;

    copystring(link, URLSIZE, href, fragment);

    if (findscheme(link))
    {

        copystring(out, size, link, cstring_length(link));

        return 1;

    }

    if (link[0] == '/' && link[1] == '/')
    {

        cstring_write_fmt(out, size, 0, "http:%s\\0", link);

        return 1;

    }

    origin = originlength(from);

    if (origin)
    {

        if (link[0] == '/')
            cstring_write_fmt(out, size, 0, "%w%s\\0", from, &origin, link);
        else if (link[0] == '?')
            cstring_write_fmt(out, size, 0, "%w%s\\0", from, &origin, link);
        else
        {

            unsigned int directory = fs_dirlength(from);

            if (directory <= origin)
                cstring_write_fmt(out, size, 0, "%w/%s\\0", from, &origin, link);
            else
                cstring_write_fmt(out, size, 0, "%w%s\\0", from, &directory, link);

        }

        out[size - 1] = '\0';

        normalize(out, origin);

        return 1;

    }

    if (findservice(link))
    {

        copystring(out, size, link, cstring_length(link));

        return 1;

    }

    if (findservice(from))
    {

        char directory[URLSIZE];

        copystring(directory, URLSIZE, from, fs_dirlength(from));

        return fs_absolute(out, size, directory, link) != 0;

    }

    return 0;

}

static unsigned int findheaderend(char *data, unsigned int count)
{

    unsigned int i;

    for (i = 0; i + 3 < count; i++)
    {

        if (data[i] == '\r' && data[i + 1] == '\n' && data[i + 2] == '\r' && data[i + 3] == '\n')
            return i + 4;

    }

    return 0;

}

static char *findheader(char *data, unsigned int end, char *name)
{

    unsigned int length = cstring_length(name);
    unsigned int i;

    for (i = 0; i + length < end; i++)
    {

        if ((i == 0 || data[i - 1] == '\n') && matchprefix(data + i, end - i, name) && data[i + length] == ':')
        {

            i += length + 1;

            while (i < end && data[i] == ' ')
                i++;

            return data + i;

        }

    }

    return 0;

}

static unsigned int headerlength(char *value)
{

    unsigned int length = 0;

    while (value[length] && value[length] != '\r' && value[length] != '\n')
        length++;

    return length;

}

static unsigned int hexvalue(char c)
{

    c = lower(c);

    if (c >= '0' && c <= '9')
        return c - '0';

    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;

    return 16;

}

static unsigned int dechunk(char *data, unsigned int count)
{

    unsigned int read = 0;
    unsigned int write = 0;

    while (read < count)
    {

        unsigned int size = 0;
        unsigned int i;

        while (read < count && hexvalue(data[read]) < 16)
            size = size * 16 + hexvalue(data[read++]);

        while (read < count && data[read] != '\n')
            read++;

        read++;

        if (!size || read >= count)
            break;

        if (size > count - read)
            size = count - read;

        for (i = 0; i < size; i++)
            data[write++] = data[read++];

        read += 2;

    }

    return write;

}

static void seterror(char *text)
{

    cstring_write_fmt(error, ERRORSIZE, 0, "%s\\0", text);

    error[ERRORSIZE - 1] = '\0';

}

static struct page page(unsigned int count, unsigned int text)
{

    struct page result;

    result.count = count;
    result.text = text;

    return result;

}

static struct page loadfile(char *url)
{

    unsigned int count = fs_read_path(1, url, source, SOURCESIZE);

    if (!count)
    {

        seterror("The file could not be opened.");

        return page(0, 0);

    }

    return page(count, !(matchsuffix(url, ".html") || matchsuffix(url, ".htm") || matchsuffix(url, ".css")));

}

static struct page loadhttp(char *url)
{

    char location[URLSIZE];
    unsigned int redirects;

    for (redirects = 0; redirects < REDIRECTS; redirects++)
    {

        unsigned int count = system_feed(0, 0, source, SOURCESIZE, "webc -url \"%s\"", url);
        unsigned int end = findheaderend(source, count);
        unsigned int status = (end > 12 && matchprefix(source, count, "http/")) ? cstring_read_value(source + 9, 3, 10) : 0;
        char *type;
        char *encoding;
        char *redirect;

        if (!count)
        {

            seterror("The host could not be reached.");

            return page(0, 0);

        }

        if (!end || !status)
        {

            seterror("The server sent something that is not HTTP.");

            return page(0, 0);

        }

        redirect = findheader(source, end, "location");

        if (status >= 300 && status < 400 && redirect)
        {

            char link[URLSIZE];

            copystring(link, URLSIZE, redirect, headerlength(redirect));

            if (!resolve(location, URLSIZE, url, link) || !issafe(location))
                break;

            url = location;

            copystring(fetched, URLSIZE, url, cstring_length(url));

            continue;

        }

        type = findheader(source, end, "content-type");
        encoding = findheader(source, end, "transfer-encoding");

        if (type && !(matchprefix(type, headerlength(type), "text/") || matchprefix(type, headerlength(type), "application/xhtml")))
        {

            seterror("This kind of content can not be shown.");

            return page(0, 0);

        }

        count -= end;

        moveback(source, end, count);

        if (encoding && matchprefix(encoding, headerlength(encoding), "chunked"))
            count = dechunk(source, count);

        return page(count, type && !matchprefix(type, headerlength(type), "text/html") && !matchprefix(type, headerlength(type), "application/xhtml"));

    }

    seterror("The page redirects too many times.");

    return page(0, 0);

}

static struct page load(char *url)
{

    unsigned int scheme = findscheme(url);

    error[0] = '\0';

    copystring(fetched, URLSIZE, url, cstring_length(url));

    if (matchprefix(url, cstring_length(url), "about:"))
    {

        unsigned int count = cstring_length(home);

        buffer_copy(source, home, count);

        return page(count, 0);

    }

    if (scheme)
    {

        if (scheme == 4 && matchprefix(url, scheme, "http"))
            return loadhttp(url);

        seterror("Only http:// addresses and files are supported.");

        return page(0, 0);

    }

    return loadfile(url);

}

static void sanitize(char *out, unsigned int size, char *in, unsigned int length)
{

    unsigned int count = 0;
    unsigned int space = 0;
    unsigned int i;

    for (i = 0; i < length && count < size - 1; i++)
    {

        char c = in[i];

        if (c == ' ' || c == '\n' || c == '\t' || c == '\r')
        {

            space = count > 0;

            continue;

        }

        if (space && count < size - 2)
            out[count++] = ' ';

        space = 0;
        out[count++] = (c == '"' || c == '\\') ? '\'' : c;

    }

    out[count] = '\0';

}

static void showerror(char *url)
{

    unsigned int count = cstring_write_fmt(source, SOURCESIZE, 0, "<html><head><title>Error</title></head><body><h1>Could not load page</h1><p>%s</p><p>Address: <code>", error);
    unsigned int i;

    for (i = 0; url[i] && count < SOURCESIZE - 64; i++)
    {

        if (url[i] == '<')
            count += cstring_write(source, SOURCESIZE, "&lt;", count);
        else if (url[i] == '&')
            count += cstring_write(source, SOURCESIZE, "&amp;", count);
        else
            source[count++] = url[i];

    }

    count += cstring_write(source, SOURCESIZE, "</code></p></body></html>", count);

    html_parse(&document, source, count);

}

static void loadstyles(void)
{

    unsigned int node;

    css_reset(&sheet);
    copystring(base, URLSIZE, current, cstring_length(current));

    for (node = 1; node < document.nodecount; node++)
    {

        struct html_node *element = &document.nodes[node];
        char *media = (element->type == HTML_NODE_ELEMENT) ? html_getattribute(&document, node, "media") : 0;

        if (media && !(matchprefix(media, cstring_length(media), "all") || matchprefix(media, cstring_length(media), "screen")))
            continue;

        if (element->tag == HTML_TAG_STYLE && element->first)
        {

            struct html_node *text = &document.nodes[element->first];

            css_parse(&sheet, html_getstring(&document, text->text), text->length, CSS_ORIGIN_AUTHOR);

        }

        if (element->tag == HTML_TAG_LINK)
        {

            char *rel = html_getattribute(&document, node, "rel");
            char *href = html_getattribute(&document, node, "href");
            char url[URLSIZE];

            if (rel && href && cstring_match_substring(rel, "stylesheet") && !cstring_match_substring(rel, "alternate") && resolve(url, URLSIZE, base, href) && issafe(url))
            {

                struct page sheetpage = load(url);

                if (sheetpage.count)
                    css_parse(&sheet, source, sheetpage.count, CSS_ORIGIN_AUTHOR);

            }

        }

        if (element->type == HTML_NODE_ELEMENT && cstring_match(html_getstring(&document, element->name), "base"))
        {

            char *href = html_getattribute(&document, node, "href");
            char url[URLSIZE];

            if (href && resolve(url, URLSIZE, current, href))
                copystring(base, URLSIZE, url, cstring_length(url));

        }

    }

}

static unsigned int createimage(struct image *image, unsigned int first)
{

    unsigned int i;

    image->target = fs_auth("mem:");
    image->parent = (image->target) ? fs_walk(1, image->target, 0, "mem:") : 0;
    image->id = 0;

    for (i = first; i < NAMES && image->parent && !image->id; i++)
    {

        cstring_write_fmt(image->name, NAMESIZE, 0, "browser%u\\0", &i);

        image->id = fs_create(1, image->target, image->parent, image->name, cstring_length(image->name));

    }

    return i;

}

static void removeimage(struct image *image)
{

    if (image->id)
        fs_remove(1, image->target, image->parent, image->id);

}

static void render(void)
{

    struct image *image = &images[nextimage];
    struct paint_canvas canvas;
    unsigned int limit;
    unsigned int height;
    unsigned char zero = 0;

    if (!wm || !viewwidth || !image->id)
        return;

    css_compute(&sheet, &document, styles, viewwidth, viewheight);
    layout_document(&layout, viewwidth);

    limit = MAXBYTES / 4 / viewwidth;
    height = (layout.height > (int)viewheight) ? (unsigned int)layout.height : viewheight;
    height = (height > limit) ? limit : height;
    height = (height > MAXHEIGHT) ? MAXHEIGHT : height;

    if (fs_write(1, image->target, image->id, &zero, 1, viewwidth * height * 4 - 1) != 1)
        return;

    canvas.pixels = fs_mmap(1, image->target, image->id, 0);
    canvas.width = viewwidth;
    canvas.height = height;

    if (!canvas.pixels)
        return;

    paint_document(&canvas, &layout);
    fs_munmap(1, image->target, image->id);
    channel_send_fmt(0, wm, MESSAGE_TYPE_WMRENDERDATA, "= page source \"mem:%s\" width \"%u\" height \"%u\"\n", image->name, &viewwidth, &height);

    nextimage = (nextimage + 1) % IMAGES;

}

static void showstate(void)
{

    unsigned int titlenode = html_gettitle(&document);
    char title[TITLESIZE];

    if (titlenode)
        sanitize(title, TITLESIZE, html_getstring(&document, document.nodes[titlenode].text), document.nodes[titlenode].length);
    else
        title[0] = '\0';

    if (title[0])
        channel_send_fmt(0, wm, MESSAGE_TYPE_WMRENDERDATA, "= window label \"%s - Browser\"\n", title);
    else
        channel_send_fmt(0, wm, MESSAGE_TYPE_WMRENDERDATA, "= window label \"Browser\"\n");

    copystring(typed, URLSIZE, current, cstring_length(current));
    channel_send_fmt(0, wm, MESSAGE_TYPE_WMRENDERDATA, "= address label \"%s\"\n= back mode \"%s\"\n= forward mode \"%s\"\n", current, (historyindex > 0) ? "normal" : "disabled", (historyindex + 1 < historycount) ? "normal" : "disabled");

}

static void pushhistory(char *url)
{

    if (historycount)
        historycount = historyindex + 1;

    if (historycount == HISTORY)
    {

        unsigned int i;

        for (i = 1; i < HISTORY; i++)
            buffer_copy(history[i - 1], history[i], URLSIZE);

        historycount--;

    }

    copystring(history[historycount], URLSIZE, url, cstring_length(url));

    historyindex = historycount;
    historycount++;

}

static void navigate(char *url, unsigned int push)
{

    char target[URLSIZE];
    struct page loaded;
    unsigned int start = 0;

    while (url[start] == ' ')
        start++;

    copystring(target, URLSIZE, url + start, cstring_length(url + start));

    if (!target[0])
        copystring(target, URLSIZE, "about:home", 10);

    if (!findscheme(target) && !findservice(target))
    {

        char full[URLSIZE];

        cstring_write_fmt(full, URLSIZE, 0, "http://%s\\0", target);
        copystring(target, URLSIZE, full, cstring_length(full));

    }

    if (push)
        pushhistory(target);

    copystring(current, URLSIZE, target, cstring_length(target));
    channel_send_fmt(0, wm, MESSAGE_TYPE_WMRENDERDATA, "= window label \"Loading - Browser\"\n");

    copystring(fetched, URLSIZE, target, cstring_length(target));

    if (issafe(target))
    {

        loaded = load(target);

    }

    else
    {

        seterror("The address contains characters that are not supported.");

        loaded = page(0, 0);

    }

    copystring(current, URLSIZE, fetched, cstring_length(fetched));
    copystring(history[historyindex], URLSIZE, current, cstring_length(current));

    if (!loaded.count)
        showerror(target);
    else if (loaded.text)
        html_parsetext(&document, source, loaded.count);
    else
        html_parse(&document, source, loaded.count);

    loadstyles();
    showstate();
    render();

}

static void loadfonts(void)
{

    unsigned int i;

    for (i = 0; i < FONT_FACES; i++)
        font_load(&faces[i], source, fs_read_path(1, fontpaths[i], source, SOURCESIZE));

}

static void onmain(struct message *message)
{

    wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        char *alfi = "initrd:data/alfi/browser.alfi";
        char *url = option_getstring("url");

        layout.document = &document;
        layout.styles = styles;
        layout.faces = faces;

        loadfonts();
        createimage(&images[1], createimage(&images[0], 0));
        channel_send(0, wm, MESSAGE_TYPE_WMRENDERFILE, cstring_length_zero(alfi), alfi);
        navigate((url[0]) ? url : "about:home", 1);
        channel_hold(0);
        channel_send(0, wm, MESSAGE_TYPE_WMUNMAP, 0, 0);
        removeimage(&images[0]);
        removeimage(&images[1]);

    }

}

static void onpath(struct message *message)
{

    if (wm)
        navigate(message->data, 1);
    else
        option_setstring("url", message->data);

}

static void onwmevent(struct message *message)
{

    struct message_wmevent *event = message_validate(MESSAGE_TYPE_WMEVENT, message->data, message->length);

    if (event)
    {

        char *data = (char *)(event + 1);

        /* the address is taken as is instead of through kv, since a URL can contain & */
        if (event->length > 14 && buffer_match(data, "q=address&url=", 14))
        {

            copystring(typed, URLSIZE, data + 14, cstring_length(data + 14));

        }

        else if (event->length > 9 && buffer_match(data, "q=go&url=", 9))
        {

            copystring(typed, URLSIZE, data + 9, cstring_length(data + 9));
            navigate(typed, 1);

        }

        else if (kv_match(event, "q=go"))
        {

            navigate(typed, 1);

        }

        else if (kv_match(event, "q=back"))
        {

            if (historyindex > 0)
                navigate(history[--historyindex], 0);

        }

        else if (kv_match(event, "q=forward"))
        {

            if (historyindex + 1 < historycount)
                navigate(history[++historyindex], 0);

        }

        else if (kv_match(event, "q=reload"))
        {

            navigate(current, 0);

        }

        else if (kv_match(event, "q=resize"))
        {

            unsigned int width = kv_getvalue(event, "width=", 10);
            unsigned int height = kv_getvalue(event, "height=", 10);

            if (width != viewwidth || height != viewheight)
            {

                viewwidth = width;
                viewheight = height;

                render();

            }

        }

        else if (kv_match(event, "q=click"))
        {

            unsigned int anchor = layout_findlink(&layout, kv_getvalue(event, "x=", 10), kv_getvalue(event, "y=", 10));
            char url[URLSIZE];

            if (anchor && resolve(url, URLSIZE, base, html_getattribute(&document, anchor, "href")))
                navigate(url, 1);

        }

    }

}

void init(void)
{

    option_add("wm-service", "wm");
    option_add("url", "");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_PATH, onpath);
    channel_bind(MESSAGE_TYPE_WMEVENT, onwmevent);

}
