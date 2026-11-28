#include <fudge.h>
#include <abi.h>
#include <compression.h>
#include <hash.h>
#include <image.h>
#include "html.h"
#include "css.h"
#include <font.h>
#include "font.h"
#include "layout.h"
#include "paint.h"
#include "svg.h"
#include "js.h"
#include "../wutils/kv.h"

#define URLSIZE                         512
#define SOURCESIZE                      0x100000
#define HISTORY                         32
#define IMAGES                          2
#define NAMES                           16
#define NAMESIZE                        32
#define TITLESIZE                       80
#define ERRORSIZE                       128
#define REDIRECTS                       5
#define MAXBYTES                        0x2000000
#define PICTURES                        1024
#define HEAPSIZE                        0x1000000
#define PAGE_HTML                       0
#define PAGE_TEXT                       1
#define PAGE_OTHER                      2
#define PICTURE_PNG                     0
#define PICTURE_SVGFILE                 1
#define PICTURE_SVGINLINE               2
#define PICTURE_CANVAS                  3

struct image
{

    unsigned int target;
    unsigned int parent;
    unsigned int id;
    char name[NAMESIZE];

};

struct store
{

    struct html_document document;
    struct css_sheet sheet;
    struct css_style styles[HTML_NODES];
    struct layout layout;
    struct js_engine js;
    struct html_document svgdocument;
    struct jpeg jpeg;
    struct font_set fonts;
    char source[SOURCESIZE];

};

struct picturesource
{

    unsigned int hash;
    unsigned int kind;
    unsigned int node;
    unsigned int source;
    unsigned int length;

};

struct page
{

    unsigned int count;
    unsigned int kind;

};

static struct html_document *document;
static struct css_sheet *sheet;
static struct css_style *styles;
static struct layout *layout;
static struct js_engine *js;
static struct font_set *fonts;
static struct image images[IMAGES];
static struct image heap;
static struct image tables;
static struct layout_picture pictures[PICTURES];
static struct picturesource sources[PICTURES];
static struct html_document *svgdocument;
static unsigned int heapbase;
static unsigned int picturecount;
static unsigned int heapused;
static struct png png;
static struct gif gif;
static struct jpeg *jpeg;
static unsigned int pngcount;
static unsigned int pngtarget;
static unsigned int pngid;
static char *source;
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
static unsigned int scrolly;
static unsigned int scheme = CSS_SCHEME_LIGHT;
static char *fontpaths[FONT_FACES] = {
    "root:data/ttf/LiberationSans-Regular.ttf",
    "root:data/ttf/LiberationSans-Bold.ttf",
    "root:data/ttf/LiberationMono-Regular.ttf",
    "root:data/ttf/LiberationMono-Bold.ttf"
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

    if (matchsuffix(url, ".png") || matchsuffix(url, ".jpg") || matchsuffix(url, ".jpeg") || matchsuffix(url, ".gif"))
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

    html_parse(document, source, count);

}

static void loadstyles(void)
{

    unsigned int node;

    css_reset(sheet);
    copystring(base, URLSIZE, current, cstring_length(current));

    for (node = 1; node < document->nodecount; node++)
    {

        struct html_node *element = &document->nodes[node];
        char *media = (element->type == HTML_NODE_ELEMENT) ? html_getattribute(document, node, "media") : 0;

        if (media && !(matchprefix(media, cstring_length(media), "all") || matchprefix(media, cstring_length(media), "screen")))
            continue;

        if (element->tag == HTML_TAG_STYLE && element->first)
        {

            struct html_node *text = &document->nodes[element->first];

            css_setbase(sheet, 0);
            css_parse(sheet, html_getstring(document, text->text), text->length, CSS_ORIGIN_AUTHOR);

        }

        if (element->tag == HTML_TAG_LINK)
        {

            char *rel = html_getattribute(document, node, "rel");
            char *href = html_getattribute(document, node, "href");
            char url[URLSIZE];

            if (rel && href && cstring_match_substring(rel, "stylesheet") && !cstring_match_substring(rel, "alternate") && resolve(url, URLSIZE, base, href) && issafe(url))
            {

                struct page sheetpage = load(url);

                /* addresses in a stylesheet are relative to the stylesheet */
                if (sheetpage.count)
                {

                    css_setbase(sheet, url);
                    css_parse(sheet, source, sheetpage.count, CSS_ORIGIN_AUTHOR);
                    css_setbase(sheet, 0);

                }

            }

        }

        if (element->type == HTML_NODE_ELEMENT && cstring_match(html_getstring(document, element->name), "base"))
        {

            char *href = html_getattribute(document, node, "href");
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

static unsigned int readpicture(void *buffer, unsigned int count, unsigned int offset)
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

static unsigned int readpng(struct png *png, void *buffer, unsigned int count, unsigned int offset)
{

    return readpicture(buffer, count, offset);

}

static unsigned int readjpeg(struct jpeg *jpeg, void *buffer, unsigned int count, unsigned int offset)
{

    return readpicture(buffer, count, offset);

}

static unsigned int readgif(struct gif *gif, void *buffer, unsigned int count, unsigned int offset)
{

    return readpicture(buffer, count, offset);

}

static unsigned int isgif(void)
{

    unsigned char magic[4];

    return readpicture(magic, 4, 0) == 4 && buffer_match(magic, "GIF8", 4);

}

static unsigned int isjpeg(void)
{

    unsigned char magic[3];

    return readpicture(magic, 3, 0) == 3 && magic[0] == 0xFF && magic[1] == 0xD8 && magic[2] == 0xFF;

}

static unsigned int decodepicture(struct layout_picture *picture)
{

    unsigned int jpg = isjpeg();
    unsigned int gifs = isgif();
    unsigned int width;
    unsigned int height;
    unsigned int size;
    unsigned int extra;
    unsigned char zero = 0;
    unsigned int *pixels;
    unsigned int y;

    if (!heap.id || !((gifs) ? gif_open(&gif, readgif) : (jpg) ? jpeg_open(jpeg, readjpeg) : png_open(&png, readpng)))
        return 0;

    width = (gifs) ? gif.width : (jpg) ? jpeg->width : png.width;
    height = (gifs) ? gif.height : (jpg) ? jpeg->height : png.height;

    if (!width || !height)
        return 0;

    size = width * height * 4;
    extra = (jpg) ? jpeg->needed : 0;

    if (width > HEAPSIZE / 4 / height || heapused + size > HEAPSIZE || extra > HEAPSIZE - heapused - size)
        return 0;

    if (fs_write(1, heap.target, heap.id, &zero, 1, heapused + size + extra - 1) != 1)
        return 0;

    pixels = fs_mmap(1, heap.target, heap.id, 0);

    if (!pixels)
        return 0;

    /* a progressive jpeg keeps its coefficients for a while in the heap after its pixels */
    if (extra)
        jpeg_decode(jpeg, (short *)(pixels + (heapused + size) / 4));

    /* a gif comes whole, as its rows can be interlaced */
    if (gifs)
    {

        y = (gif_decode(&gif, pixels + heapused / 4)) ? height : 0;

    }

    else
    {

        for (y = 0; y < height; y++)
        {

            unsigned int *row = pixels + heapused / 4 + y * width;

            if (!((jpg) ? jpeg_readrow(jpeg, row) : png_readrow(&png, row)))
                break;

        }

    }

    fs_munmap(1, heap.target, heap.id);

    if (y < height)
        return 0;

    picture->width = width;
    picture->height = height;
    picture->ratiowidth = width;
    picture->ratioheight = height;
    picture->columns = width;
    picture->rows = height;
    picture->offset = heapused;
    heapused += size;

    return 1;

}

static unsigned int issvg(char *data, unsigned int count)
{

    unsigned int i;

    for (i = 0; i + 4 < count && i < 0x400; i++)
    {

        if (buffer_match(data + i, "<svg", 4))
            return 1;

    }

    return 0;

}

static void setsvgpicture(struct layout_picture *picture, struct svg_size *size)
{

    picture->width = size->width;
    picture->height = size->height;
    picture->ratiowidth = size->ratiowidth;
    picture->ratioheight = size->ratioheight;
    picture->offset = 0;
    picture->columns = 0;
    picture->rows = 0;

}

static unsigned int storesvg(struct layout_picture *picture, struct picturesource *info, unsigned int count)
{

    unsigned char zero = 0;
    struct svg_size size;
    unsigned int root;
    char *data;

    html_parse(svgdocument, source, count);

    root = svg_findroot(svgdocument);

    if (!root || !heap.id || heapused + count + 4 > HEAPSIZE)
        return 0;

    if (fs_write(1, heap.target, heap.id, &zero, 1, heapused + count - 1) != 1)
        return 0;

    data = fs_mmap(1, heap.target, heap.id, 0);

    if (!data)
        return 0;

    buffer_copy(data + heapused, source, count);
    fs_munmap(1, heap.target, heap.id);

    size = svg_measure(svgdocument, root, 16);

    setsvgpicture(picture, &size);

    info->kind = PICTURE_SVGFILE;
    info->source = heapused;
    info->length = count;
    heapused = (heapused + count + 3) & ~3;

    return 1;

}

static unsigned int readdata(char *url)
{

    unsigned int length = cstring_length(url);
    unsigned int comma = buffer_findbyte(url, length, ',');
    unsigned int count = 0;
    unsigned int i;

    if (comma >= length)
        return 0;

    if (comma >= 12 && buffer_match(url + comma - 7, ";base64", 7))
        return (length - comma - 1 < SOURCESIZE / 4 * 3) ? base64_decode((unsigned char *)url + comma + 1, length - comma - 1, (unsigned char *)source) : 0;

    for (i = comma + 1; i < length && count < SOURCESIZE; i++)
    {

        if (url[i] == '%' && i + 2 < length && hexvalue(url[i + 1]) < 16 && hexvalue(url[i + 2]) < 16)
        {

            source[count++] = hexvalue(url[i + 1]) * 16 + hexvalue(url[i + 2]);
            i += 2;

        }

        else
        {

            source[count++] = url[i];

        }

    }

    return count;

}

static unsigned int loadpicture(char *url)
{

    unsigned int hash = djb_hash(cstring_length(url), url);
    struct page loaded;
    unsigned int i;

    for (i = 0; i < picturecount; i++)
    {

        if (sources[i].kind != PICTURE_SVGINLINE && sources[i].kind != PICTURE_CANVAS && sources[i].hash == hash)
            return i + 1;

    }

    if (picturecount >= PICTURES)
        return 0;

    if (buffer_match(url, "data:", 5))
    {

        pngcount = readdata(url);
        pngid = 0;

    }

    else if (findscheme(url))
    {

        loaded = load(url);
        pngcount = loaded.count;
        pngid = 0;

    }

    else
    {

        pngtarget = fs_auth(url);
        pngid = (pngtarget) ? fs_walk(1, pngtarget, 0, url) : 0;
        pngcount = (pngid) ? fs_read_full(1, pngtarget, pngid, source, SOURCESIZE, 0) : 0;

        /* a file that fits is read from memory, a larger one is streamed */
        if (pngcount < SOURCESIZE)
            pngid = 0;

    }

    if (!pngcount)
        return 0;

    sources[picturecount].hash = hash;
    sources[picturecount].kind = PICTURE_PNG;

    if (!pngid && issvg(source, pngcount))
    {

        if (!storesvg(&pictures[picturecount], &sources[picturecount], pngcount))
            return 0;

    }

    else if (!decodepicture(&pictures[picturecount]))
    {

        return 0;

    }

    picturecount++;

    return picturecount;

}

static unsigned int insvg(unsigned int node)
{

    for (node = document->nodes[node].parent; node; node = document->nodes[node].parent)
    {

        if (document->nodes[node].tag == HTML_TAG_SVG)
            return 1;

    }

    return 0;

}

static unsigned int canvassize(unsigned int node, char *name, unsigned int fallback)
{

    char *text = html_getattribute(document, node, name);

    return (text && cstring_isdigit(text[0])) ? cstring_read_value(text, cstring_length(text), 10) : fallback;

}

static unsigned int addcanvas(unsigned int node)
{

    struct svg_size size;

    /* a canvas is a picture of its own size that scripts draw on, painted from what they drew */
    if (picturecount >= PICTURES)
        return 0;

    size.width = canvassize(node, "width", 300);
    size.height = canvassize(node, "height", 150);
    size.ratiowidth = size.width;
    size.ratioheight = size.height;

    setsvgpicture(&pictures[picturecount], &size);

    sources[picturecount].hash = 0;
    sources[picturecount].kind = PICTURE_CANVAS;
    sources[picturecount].node = node;
    picturecount++;

    return picturecount;

}

static unsigned int addinlinesvg(unsigned int node)
{

    struct svg_size size;

    if (picturecount >= PICTURES || insvg(node))
        return 0;

    size = svg_measure(document, node, 16);

    setsvgpicture(&pictures[picturecount], &size);

    sources[picturecount].hash = 0;
    sources[picturecount].kind = PICTURE_SVGINLINE;
    sources[picturecount].node = node;
    picturecount++;

    return picturecount;

}

static unsigned int loadbackground(unsigned int node)
{

    char *address = css_backgroundurl(sheet, &styles[node]);
    char *relative = css_backgroundbase(sheet, &styles[node]);
    char url[URLSIZE];

    if (!address)
        return 0;

    if (buffer_match(address, "data:", 5))
        return loadpicture(address);

    return (resolve(url, URLSIZE, (relative) ? relative : base, address) && issafe(url)) ? loadpicture(url) : 0;

}

static void loadpictures(void)
{

    unsigned int node;

    picturecount = 0;
    heapused = 0;

    buffer_clear(layout->picture, sizeof (unsigned short) * HTML_NODES);
    buffer_clear(layout->background, sizeof (unsigned short) * HTML_NODES);

    /* background images are named in the styles, so they are computed first */
    css_compute(sheet, document, styles, viewwidth, viewheight, scheme);

    for (node = 1; node < document->nodecount; node++)
    {

        char *src = (document->nodes[node].tag == HTML_TAG_IMG) ? html_getattribute(document, node, "src") : 0;
        char url[URLSIZE];

        if (src && buffer_match(src, "data:", 5))
            layout->picture[node] = loadpicture(src);
        else if (src && resolve(url, URLSIZE, base, src) && issafe(url))
            layout->picture[node] = loadpicture(url);
        else if (document->nodes[node].tag == HTML_TAG_SVG)
            layout->picture[node] = addinlinesvg(node);
        else if (document->nodes[node].tag == HTML_TAG_CANVAS)
            layout->picture[node] = addcanvas(node);

        layout->background[node] = (document->nodes[node].type == HTML_NODE_ELEMENT) ? loadbackground(node) : 0;

    }

    heapbase = heapused;

}

static unsigned int placesvg(struct layout_picture *picture, unsigned int node, unsigned int needed)
{

    struct paint_area area = paint_getcontent(layout, node);
    unsigned int size;

    if (!layout->boxes[node].placed || area.w <= 0 || area.h <= 0 || area.w > 4096 || area.h > 4096)
        return needed;

    size = (unsigned int)area.w * (unsigned int)area.h * 4;

    if (needed + size > HEAPSIZE)
        return needed;

    picture->offset = needed;
    picture->columns = area.w;
    picture->rows = area.h;

    return needed + size;

}

static void rasterize(void)
{

    unsigned int needed = heapbase;
    unsigned char zero = 0;
    unsigned int *pixels;
    unsigned int node;
    unsigned int i;

    for (i = 0; i < picturecount; i++)
    {

        if (sources[i].kind != PICTURE_PNG)
            pictures[i].columns = 0;

    }

    /* each svg is drawn once at the size of its first box, a second box of another size scales it */
    for (node = 1; node < document->nodecount; node++)
    {

        unsigned int index = layout->picture[node];

        if (index && sources[index - 1].kind != PICTURE_PNG && !pictures[index - 1].columns)
            needed = placesvg(&pictures[index - 1], node, needed);

    }

    heapused = needed;

    if (needed == heapbase)
        return;

    pixels = (fs_write(1, heap.target, heap.id, &zero, 1, needed - 1) == 1) ? fs_mmap(1, heap.target, heap.id, 0) : 0;

    for (i = 0; i < picturecount; i++)
    {

        struct layout_picture *picture = &pictures[i];
        struct svg_canvas canvas;

        if (sources[i].kind == PICTURE_PNG || !picture->columns)
            continue;

        if (!pixels)
        {

            picture->columns = 0;

            continue;

        }

        canvas.pixels = pixels + picture->offset / 4;
        canvas.width = picture->columns;
        canvas.height = picture->rows;

        buffer_clear(canvas.pixels, canvas.width * canvas.height * 4);

        if (sources[i].kind == PICTURE_SVGFILE)
        {

            html_parse(svgdocument, (char *)pixels + sources[i].source, sources[i].length);
            svg_render(&canvas, svgdocument, 0, svg_findroot(svgdocument), 0xFF000000, 0);

        }

        else if (sources[i].kind == PICTURE_CANVAS)
        {

            svg_rendercanvas(&canvas, js->canvasops, js->canvasopcount, sources[i].node, canvassize(sources[i].node, "width", 300), canvassize(sources[i].node, "height", 150), js->strings, fonts);

        }

        else
        {

            svg_render(&canvas, document, sheet, sources[i].node, styles[sources[i].node].color, styles[sources[i].node].fill);

        }

    }

    if (pixels)
        fs_munmap(1, heap.target, heap.id);
    else
        heapused = heapbase;

}

static void paintpage(void)
{

    struct image *image = &images[nextimage];
    struct paint_canvas canvas;
    unsigned int limit;
    unsigned int height;
    unsigned char zero = 0;

    if (!wm || !viewwidth || !viewheight || !image->id)
        return;

    limit = MAXBYTES / 4 / viewwidth;
    height = (viewheight > limit) ? limit : viewheight;

    if (fs_write(1, image->target, image->id, &zero, 1, viewwidth * height * 4 - 1) != 1)
        return;

    canvas.pixels = fs_mmap(1, image->target, image->id, 0);
    canvas.width = viewwidth;
    canvas.height = height;
    canvas.top = scrolly;
    canvas.scratch = 0;
    canvas.scratchsize = 0;

    /* the heap grows by scratch space for drawing transformed boxes, after the pictures in it */
    if (heap.id && heapused + PAINT_SCRATCH * 4 <= HEAPSIZE && fs_write(1, heap.target, heap.id, &zero, 1, heapused + PAINT_SCRATCH * 4 - 1) == 1)
        canvas.scratchsize = PAINT_SCRATCH;

    canvas.heap = (heapused || canvas.scratchsize) ? fs_mmap(1, heap.target, heap.id, 0) : 0;

    if (canvas.heap && canvas.scratchsize)
        canvas.scratch = canvas.heap + heapused / 4;

    if (canvas.pixels)
        paint_document(&canvas, layout);

    if (canvas.heap)
        fs_munmap(1, heap.target, heap.id);

    if (!canvas.pixels)
        return;

    fs_munmap(1, image->target, image->id);
    channel_send_fmt(0, wm, MESSAGE_TYPE_DATA, "= page source \"mem:%s\" width \"%u\" height \"%u\"\n", image->name, &viewwidth, &height);

    nextimage = (nextimage + 1) % IMAGES;

}

static void render(void)
{

    unsigned int extent;

    if (!wm || !viewwidth || !viewheight)
        return;

    css_compute(sheet, document, styles, viewwidth, viewheight, scheme);
    layout_document(layout, viewwidth, viewheight);

    extent = (layout->height > (int)viewheight) ? (unsigned int)layout->height : viewheight;

    if (scrolly > extent - viewheight)
        scrolly = extent - viewheight;

    channel_send_fmt(0, wm, MESSAGE_TYPE_DATA, "= frame extent \"%u\"\n", &extent);
    rasterize();
    paintpage();

}

static void showstate(void)
{

    unsigned int titlenode = html_gettitle(document);
    char title[TITLESIZE];

    if (titlenode)
        sanitize(title, TITLESIZE, html_getstring(document, document->nodes[titlenode].text), document->nodes[titlenode].length);
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

    char *type = html_getattribute(document, node, "type");

    if (document->nodes[node].tag != HTML_TAG_SCRIPT)
        return 0;

    return !type || !type[0] || matchsuffix(type, "javascript") || matchsuffix(type, "ecmascript");

}

static struct js_rect measurenode(unsigned int node)
{

    struct js_rect rect;

    rect.x = (node < HTML_NODES && layout->boxes[node].placed) ? layout->boxes[node].x : 0;
    rect.y = (node < HTML_NODES && layout->boxes[node].placed) ? layout->boxes[node].y : 0;
    rect.w = (node < HTML_NODES && layout->boxes[node].placed) ? layout->boxes[node].w : 0;
    rect.h = (node < HTML_NODES && layout->boxes[node].placed) ? layout->boxes[node].h : 0;

    return rect;

}

static void runscripts(void)
{

    unsigned int count = document->nodecount;
    unsigned int node;

    js_reset(js, document, scheme == CSS_SCHEME_DARK, system_unixtime("clock:0"));
    js_setview(js, viewwidth, viewheight, measurenode);

    for (node = 1; node < count; node++)
    {

        char *src;
        char url[URLSIZE];

        if (!isscript(node))
            continue;

        src = html_getattribute(document, node, "src");

        if (src)
        {

            if (resolve(url, URLSIZE, base, src) && issafe(url))
            {

                struct page script = load(url);

                if (script.count)
                    js_run(js, source, script.count);

            }

        }

        else if (document->nodes[node].first)
        {

            struct html_node *text = &document->nodes[document->nodes[node].first];

            js_run(js, html_getstring(document, text->text), text->length);

        }

    }

    js_dispatch(js, 0, "DOMContentLoaded");
    js_dispatch(js, 0, "load");
    js_runtimers(js);

}

static unsigned int istag(unsigned int node, char *name)
{

    return node && document->nodes[node].type == HTML_NODE_ELEMENT && cstring_match(html_getstring(document, document->nodes[node].name), name);

}

static unsigned int clicknode(unsigned int node)
{

    unsigned int handled = js_click(js, node);

    /* timers set by the listeners run now, as they do after the scripts on load */
    if (handled)
        js_runtimers(js);

    while (node)
    {

        unsigned int parent = document->nodes[node].parent;

        if (istag(node, "summary") && istag(parent, "details"))
        {

            if (html_getattribute(document, parent, "open"))
                html_removeattribute(document, parent, "open");
            else
                html_setattribute(document, parent, "open", "open");

            handled = 1;

        }

        node = parent;

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

        if (loaded.count > 8 && (buffer_match(source, "\x89PNG", 4) || buffer_match(source, "\xFF\xD8\xFF", 3)))
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
        html_parsetext(document, source, loaded.count);
    else
        html_parse(document, source, loaded.count);

    loadstyles();
    runscripts();
    loadpictures();
    showstate();

    scrolly = 0;

    channel_send_fmt(0, wm, MESSAGE_TYPE_DATA, "= frame scroll \"0\"\n");
    render();

}

static void loadfonts(void)
{

    unsigned int i;

    /* the outlines are read while drawing, so every font keeps its own buffer */
    for (i = 0; i < FONT_FACES; i++)
        font_load(fonts, i, fs_read_path(1, fontpaths[i], fonts->faces[i].data, FONT_DATA));

}

static unsigned int mapstore(struct image *image)
{

    struct store *store = (image->id) ? fs_mmap(1, image->target, image->id, sizeof (struct store)) : 0;

    if (!store)
        return 0;

    document = &store->document;
    sheet = &store->sheet;
    styles = store->styles;
    layout = &store->layout;
    js = &store->js;
    svgdocument = &store->svgdocument;
    jpeg = &store->jpeg;
    fonts = &store->fonts;
    source = store->source;

    return 1;

}

static void onmain(struct message *message)
{

    wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        char *alfi = "initrd:data/alfi/browser.alfi";
        char *url = option_getstring("url");

        createimage(&tables, createimage(&heap, createimage(&images[1], createimage(&images[0], 0))));

        if (mapstore(&tables))
        {

            layout->document = document;
            layout->styles = styles;
            layout->gradients = sheet->gradients;
            layout->grids = sheet->grids;
            layout->fonts = fonts;
            layout->pictures = pictures;

            loadfonts();
            channel_send(0, wm, MESSAGE_TYPE_WMRENDERFILE, cstring_length_zero(alfi), alfi);

            if (cstring_match(option_getstring("scheme"), "dark"))
                scheme = CSS_SCHEME_DARK;

            showscheme();
            navigate((url[0]) ? url : "about:home", 1);
            channel_hold(0);
            channel_send(0, wm, MESSAGE_TYPE_WMUNMAP, 0, 0);
            fs_munmap(1, tables.target, tables.id);

        }

        removeimage(&images[0]);
        removeimage(&images[1]);
        removeimage(&heap);
        removeimage(&tables);

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
            js->dark = scheme == CSS_SCHEME_DARK;

            js_dispatch(js, JS_MEDIA, "change");
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

        else if (kv_match(event, "q=scroll"))
        {

            unsigned int y = kv_getvalue(event, "y=", 10);

            if (y != scrolly)
            {

                scrolly = y;

                paintpage();

            }

        }

        else if (kv_match(event, "q=click"))
        {

            unsigned int x = kv_getvalue(event, "x=", 10);
            unsigned int y = kv_getvalue(event, "y=", 10);
            unsigned int anchor = layout_findlink(layout, x, y);
            char url[URLSIZE];

            if (clicknode(layout_findnode(layout, x, y)))
                render();
            else if (anchor && resolve(url, URLSIZE, base, html_getattribute(document, anchor, "href")))
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

