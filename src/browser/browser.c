#include <fudge.h>
#include <abi.h>
#include <compression.h>
#include <hash.h>
#include <image.h>
#include "html.h"
#include "css.h"
#include "font.h"
#include "layout.h"
#include "paint.h"
#include "js.h"
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
#define PICTURES                        64
#define HEAPSIZE                        0x1000000
#define PAGE_HTML                       0
#define PAGE_TEXT                       1
#define PAGE_OTHER                      2

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
    unsigned int kind;

};

static struct html_document document;
static struct css_sheet sheet;
static struct css_style styles[HTML_NODES];
static struct layout layout;
static struct js_engine js;
static struct font_face faces[FONT_FACES];
static struct image images[IMAGES];
static struct image heap;
static struct layout_picture pictures[PICTURES];
static unsigned int picturehashes[PICTURES];
static unsigned int picturecount;
static unsigned int heapused;
static struct png png;
static unsigned int pngcount;
static unsigned int pngtarget;
static unsigned int pngid;
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
static unsigned int scheme = CSS_SCHEME_LIGHT;
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
    "@media (prefers-color-scheme: dark){body{color:#dde2de;background:#151816}h1{color:#9fd3a8;border-color:#3d5e45}code{background:#262b28}a{color:#8cc4ff}}"
    "</style></head><body>"
    "<h1>Browser</h1>"
    "<p>Type an address in the bar above and press Enter. Addresses can be files, such as <code>root:data/html/index.html</code>, or pages over <code>http://</code> and <code>https://</code>.</p>"
    "<ul><li><a href=\"initrd:data/help.txt\">initrd:data/help.txt</a></li><li><a href=\"root:data/html/index.html\">root:data/html/index.html</a> (needs the root disk)</li></ul>"
    "<p>HTML, CSS, PNG images and simple JavaScript are supported.</p>"
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

        unsigned int scheme = findscheme(from);

        if (scheme)
            cstring_write_fmt(out, size, 0, "%w:%s\\0", from, &scheme, link);
        else
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

static struct page page(unsigned int count, unsigned int kind)
{

    struct page result;

    result.count = count;
    result.kind = kind;

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

    if (matchsuffix(url, ".html") || matchsuffix(url, ".htm"))
        return page(count, PAGE_HTML);

    if (matchsuffix(url, ".png"))
        return page(count, PAGE_OTHER);

    return page(count, PAGE_TEXT);

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
        unsigned int kind;
        unsigned int chunked;

        if (!count)
        {

            seterror((findscheme(url) == 5) ? "The host could not be reached, or its certificate could not be verified." : "The host could not be reached.");

            return page(0, 0);

        }

        if (!end || !status)
        {

            seterror("The server sent something that is not HTTP.");

            return page(0, 0);

        }

        redirect = findheader(source, end, "location");

        if (status >= 300 && status < 400 && status != 304)
        {

            char link[URLSIZE];
            char next[URLSIZE];

            if (!redirect)
            {

                seterror("The server redirected without saying where to.");

                return page(0, 0);

            }

            copystring(link, URLSIZE, redirect, headerlength(redirect));

            if (!resolve(next, URLSIZE, url, link) || !issafe(next))
            {

                seterror("The server redirected to an address that can not be used.");

                return page(0, 0);

            }

            copystring(fetched, URLSIZE, next, cstring_length(next));

            if (!matchprefix(next, cstring_length(next), "http://") && !matchprefix(next, cstring_length(next), "https://"))
            {

                seterror("The page has moved to an address that is not http:// or https://.");

                return page(0, 0);

            }

            copystring(location, URLSIZE, next, cstring_length(next));

            url = location;

            continue;

        }

        type = findheader(source, end, "content-type");
        encoding = findheader(source, end, "transfer-encoding");
        chunked = encoding && matchprefix(encoding, headerlength(encoding), "chunked");
        kind = PAGE_HTML;

        if (type && !(matchprefix(type, headerlength(type), "text/html") || matchprefix(type, headerlength(type), "application/xhtml")))
            kind = (matchprefix(type, headerlength(type), "text/")) ? PAGE_TEXT : PAGE_OTHER;

        /* the headers are overwritten by the body, so everything read from them comes first */
        count -= end;

        moveback(source, end, count);

        if (chunked)
            count = dechunk(source, count);

        return page(count, kind);

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

        if ((scheme == 4 && matchprefix(url, scheme, "http")) || (scheme == 5 && matchprefix(url, scheme, "https")))
            return loadhttp(url);

        seterror("Only http:// and https:// addresses and files are supported.");

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

static unsigned int readpng(struct png *png, void *buffer, unsigned int count, unsigned int offset)
{

    if (pngid)
        return fs_read_full(1, pngtarget, pngid, buffer, count, offset);

    if (offset >= pngcount)
        return 0;

    if (count > pngcount - offset)
        count = pngcount - offset;

    buffer_copy(buffer, source + offset, count);

    return count;

}

static unsigned int decodepicture(struct layout_picture *picture)
{

    unsigned int size;
    unsigned char zero = 0;
    unsigned int *pixels;
    unsigned int y;

    if (!heap.id || !png_open(&png, readpng) || !png.width || !png.height)
        return 0;

    size = png.width * png.height * 4;

    if (png.width > HEAPSIZE / 4 / png.height || heapused + size > HEAPSIZE)
        return 0;

    if (fs_write(1, heap.target, heap.id, &zero, 1, heapused + size - 1) != 1)
        return 0;

    pixels = fs_mmap(1, heap.target, heap.id, 0);

    if (!pixels)
        return 0;

    for (y = 0; y < png.height; y++)
    {

        if (!png_readrow(&png, pixels + heapused / 4 + y * png.width))
            break;

    }

    fs_munmap(1, heap.target, heap.id);

    if (y < png.height)
        return 0;

    picture->width = png.width;
    picture->height = png.height;
    picture->offset = heapused;
    heapused += size;

    return 1;

}

static unsigned int loadpicture(char *url)
{

    unsigned int hash = djb_hash(cstring_length(url), url);
    struct page loaded;
    unsigned int i;

    for (i = 0; i < picturecount; i++)
    {

        if (picturehashes[i] == hash)
            return i + 1;

    }

    if (picturecount >= PICTURES)
        return 0;

    if (findscheme(url))
    {

        loaded = load(url);
        pngcount = loaded.count;
        pngid = 0;

        if (!pngcount)
            return 0;

    }

    else
    {

        pngtarget = fs_auth(url);
        pngid = (pngtarget) ? fs_walk(1, pngtarget, 0, url) : 0;

        if (!pngid)
            return 0;

    }

    if (!decodepicture(&pictures[picturecount]))
        return 0;

    picturehashes[picturecount] = hash;
    picturecount++;

    return picturecount;

}

static void loadpictures(void)
{

    unsigned int node;

    picturecount = 0;
    heapused = 0;

    buffer_clear(layout.picture, sizeof (unsigned short) * HTML_NODES);

    for (node = 1; node < document.nodecount; node++)
    {

        char *src = (document.nodes[node].tag == HTML_TAG_IMG) ? html_getattribute(&document, node, "src") : 0;
        char url[URLSIZE];

        if (src && resolve(url, URLSIZE, base, src) && issafe(url))
            layout.picture[node] = loadpicture(url);

    }

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

    css_compute(&sheet, &document, styles, viewwidth, viewheight, scheme);
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
    canvas.heap = (heapused) ? fs_mmap(1, heap.target, heap.id, 0) : 0;

    if (canvas.pixels)
        paint_document(&canvas, &layout);

    if (canvas.heap)
        fs_munmap(1, heap.target, heap.id);

    if (!canvas.pixels)
        return;

    fs_munmap(1, image->target, image->id);
    channel_send_fmt(0, wm, MESSAGE_TYPE_DATA, "= page source \"mem:%s\" width \"%u\" height \"%u\"\n", image->name, &viewwidth, &height);

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
        channel_send_fmt(0, wm, MESSAGE_TYPE_DATA, "= window label \"%s - Browser\"\n", title);
    else
        channel_send_fmt(0, wm, MESSAGE_TYPE_DATA, "= window label \"Browser\"\n");

    copystring(typed, URLSIZE, current, cstring_length(current));
    channel_send_fmt(0, wm, MESSAGE_TYPE_DATA, "= address label \"%s\"\n= back mode \"%s\"\n= forward mode \"%s\"\n", current, (historyindex > 0) ? "normal" : "disabled", (historyindex + 1 < historycount) ? "normal" : "disabled");

}

static void showscheme(void)
{

    channel_send_fmt(0, wm, MESSAGE_TYPE_DATA, "= scheme label \"%s\"\n", (scheme == CSS_SCHEME_DARK) ? "Light" : "Dark");

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

static unsigned int isscript(unsigned int node)
{

    char *type = html_getattribute(&document, node, "type");

    if (document.nodes[node].tag != HTML_TAG_SCRIPT)
        return 0;

    return !type || !type[0] || matchsuffix(type, "javascript") || matchsuffix(type, "ecmascript");

}

static void runscripts(void)
{

    unsigned int count = document.nodecount;
    unsigned int node;

    js_reset(&js, &document, scheme == CSS_SCHEME_DARK);

    for (node = 1; node < count; node++)
    {

        char *src;
        char url[URLSIZE];

        if (!isscript(node))
            continue;

        src = html_getattribute(&document, node, "src");

        if (src)
        {

            if (resolve(url, URLSIZE, base, src) && issafe(url))
            {

                struct page script = load(url);

                if (script.count)
                    js_run(&js, source, script.count);

            }

        }

        else if (document.nodes[node].first)
        {

            struct html_node *text = &document.nodes[document.nodes[node].first];

            js_run(&js, html_getstring(&document, text->text), text->length);

        }

    }

    js_dispatch(&js, 0, "DOMContentLoaded");
    js_dispatch(&js, 0, "load");

}

static unsigned int clicknode(unsigned int node)
{

    unsigned int handled = 0;

    while (node)
    {

        if (js_haslistener(&js, node, "click"))
        {

            js_dispatch(&js, node, "click");

            handled = 1;

        }

        node = document.nodes[node].parent;

    }

    return handled;

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
    channel_send_fmt(0, wm, MESSAGE_TYPE_DATA, "= window label \"Loading - Browser\"\n");

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

    if (loaded.count && loaded.kind == PAGE_OTHER)
    {

        if (loaded.count > 8 && buffer_match(source, "\x89PNG", 4))
        {

            loaded.count = cstring_write_fmt(source, SOURCESIZE, 0, "<html><body style=\"margin:0\"><img src=\"%s\"></body></html>", current);
            loaded.kind = PAGE_HTML;

        }

        else
        {

            seterror("This kind of content can not be shown.");

            loaded.count = 0;

        }

    }

    if (!loaded.count)
        showerror(target);
    else if (loaded.kind == PAGE_TEXT)
        html_parsetext(&document, source, loaded.count);
    else
        html_parse(&document, source, loaded.count);

    loadstyles();
    runscripts();
    loadpictures();
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
        layout.gradients = sheet.gradients;
        layout.faces = faces;
        layout.pictures = pictures;

        loadfonts();
        createimage(&heap, createimage(&images[1], createimage(&images[0], 0)));
        channel_send(0, wm, MESSAGE_TYPE_WMRENDERFILE, cstring_length_zero(alfi), alfi);

        if (cstring_match(option_getstring("scheme"), "dark"))
            scheme = CSS_SCHEME_DARK;

        showscheme();
        navigate((url[0]) ? url : "about:home", 1);
        channel_hold(0);
        channel_send(0, wm, MESSAGE_TYPE_WMUNMAP, 0, 0);
        removeimage(&images[0]);
        removeimage(&images[1]);
        removeimage(&heap);

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

        else if (kv_match(event, "q=scheme"))
        {

            scheme = (scheme == CSS_SCHEME_DARK) ? CSS_SCHEME_LIGHT : CSS_SCHEME_DARK;
            js.dark = scheme == CSS_SCHEME_DARK;

            js_dispatch(&js, JS_MEDIA, "change");
            showscheme();
            render();

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

            unsigned int x = kv_getvalue(event, "x=", 10);
            unsigned int y = kv_getvalue(event, "y=", 10);
            unsigned int anchor = layout_findlink(&layout, x, y);
            char url[URLSIZE];

            if (clicknode(layout_findnode(&layout, x, y)))
                render();
            else if (anchor && resolve(url, URLSIZE, base, html_getattribute(&document, anchor, "href")))
                navigate(url, 1);

        }

    }

}

void init(void)
{

    option_add("wm-service", "wm");
    option_add("url", "");
    option_add("scheme", "light");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_PATH, onpath);
    channel_bind(MESSAGE_TYPE_WMEVENT, onwmevent);

}
