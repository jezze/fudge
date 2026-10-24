#include <fudge.h>
#include "resource.h"
#include "mmap.h"
#include "mailbox.h"
#include "task.h"
#include "node.h"
#include "pool.h"
#include "service.h"
#include "kernel.h"
#include "memsrv.h"

#define ROOT                            1
#define FILEPAGES                       (MEMSRV_FILESIZE / MEMSRV_PAGESIZE)
#define NONE                            0xFFFF

struct file
{

    unsigned int used;
    unsigned int linked;
    unsigned int tasks;
    unsigned int size;
    unsigned int first;
    unsigned int last;
    unsigned int cachen;
    unsigned int cachepage;
    unsigned int length;
    char name[MEMSRV_NAMESIZE];

};

static struct node_operands operands;
static struct service service;
static struct spinlock spinlock;
static struct file files[MEMSRV_FILES];
static unsigned short next[MEMSRV_PAGES];
static unsigned int freepages;
static unsigned int nfree;
static unsigned long base;
static void (*unmapcallback)(unsigned long vaddress, unsigned int size);
static unsigned int inode;

static struct file *getfile(unsigned int id)
{

    return (id > ROOT && id - ROOT - 1 < MEMSRV_FILES && files[id - ROOT - 1].linked) ? &files[id - ROOT - 1] : 0;

}

static unsigned int getid(struct file *file)
{

    return file - files + ROOT + 1;

}

static unsigned int countpages(unsigned int size)
{

    return (size + MEMSRV_PAGESIZE - 1) / MEMSRV_PAGESIZE;

}

static unsigned char *getpage(struct file *file, unsigned int offset)
{

    unsigned int n = offset / MEMSRV_PAGESIZE;
    unsigned int page = file->first;
    unsigned int i = 0;

    if (file->cachepage != NONE && file->cachen <= n)
    {

        page = file->cachepage;
        i = file->cachen;

    }

    for (; i < n; i++)
        page = next[page];

    file->cachen = n;
    file->cachepage = page;

    return (unsigned char *)(base + page * MEMSRV_PAGESIZE) + offset % MEMSRV_PAGESIZE;

}

static unsigned int grow(struct file *file, unsigned int size)
{

    unsigned int have = countpages(file->size);
    unsigned int need = countpages(size);

    if (need > FILEPAGES || need - have > nfree)
        return 0;

    for (; have < need; have++)
    {

        unsigned int page = freepages;

        freepages = next[page];
        next[page] = NONE;
        nfree--;

        if (have)
            next[file->last] = page;
        else
            file->first = page;

        file->last = page;

        buffer_clear((void *)(base + page * MEMSRV_PAGESIZE), MEMSRV_PAGESIZE);

    }

    file->size = size;

    return 1;

}

static unsigned int readdata(struct file *file, unsigned char *buffer, unsigned int count, unsigned int offset)
{

    unsigned int done = 0;

    if (offset >= file->size)
        return 0;

    if (count > file->size - offset)
        count = file->size - offset;

    while (done < count)
    {

        unsigned int chunk = MEMSRV_PAGESIZE - (offset + done) % MEMSRV_PAGESIZE;

        if (chunk > count - done)
            chunk = count - done;

        buffer_copy(buffer + done, getpage(file, offset + done), chunk);

        done += chunk;

    }

    return count;

}

static unsigned int writedata(struct file *file, unsigned char *buffer, unsigned int count, unsigned int offset)
{

    unsigned int done = 0;

    if (!count || offset + count < offset)
        return 0;

    if (offset + count > file->size && !grow(file, offset + count))
        return 0;

    while (done < count)
    {

        unsigned int chunk = MEMSRV_PAGESIZE - (offset + done) % MEMSRV_PAGESIZE;

        if (chunk > count - done)
            chunk = count - done;

        buffer_copy(getpage(file, offset + done), buffer + done, chunk);

        done += chunk;

    }

    return count;

}

static struct mmap_entry *findmapping(struct mmap_header *header, unsigned int id)
{

    unsigned int i;

    for (i = 0; i < header->nentries; i++)
    {

        struct mmap_entry *entry = &header->entries[i];

        if (entry->type == MMAP_TYPE_MEM && entry->paddress == id)
            return entry;

    }

    return 0;

}

static struct mmap_entry *findoverlap(struct mmap_header *header, unsigned long vaddress, unsigned int size)
{

    unsigned int i;

    for (i = 0; i < header->nentries; i++)
    {

        struct mmap_entry *entry = &header->entries[i];

        if (entry->type == MMAP_TYPE_MEM && entry->vaddress < vaddress + size && vaddress < entry->vaddress + entry->size)
            return entry;

    }

    return 0;

}

static unsigned long findgap(struct mmap_header *header, unsigned int size)
{

    unsigned long vaddress = KERNEL_VMEM;
    struct mmap_entry *entry;

    while (vaddress + size <= KERNEL_VBUFFER)
    {

        if (!(entry = findoverlap(header, vaddress, size)))
            return vaddress;

        vaddress = entry->vaddress + entry->size;

    }

    return 0;

}

static struct mailbox *getmailbox(unsigned int source)
{

    struct resource *resource = pool_getnoderesource(source);

    return (resource && resource->type == RESOURCE_MAILBOX) ? resource->data : 0;

}

static void release(struct file *file)
{

    if (file->used && !file->linked && !file->tasks)
    {

        if (file->size)
        {

            next[file->last] = freepages;
            freepages = file->first;
            nfree += countpages(file->size);

        }

        file->size = 0;
        file->cachepage = NONE;
        file->used = 0;

    }

}

static unsigned int walk(unsigned int id, char *path, unsigned int length)
{

    unsigned int offset = 0;

    while (offset < length)
    {

        char *cp = path + offset;
        unsigned int cl = buffer_findbyte(cp, length - offset, '/');
        unsigned int i;

        if (cl == 0 || (cl == 2 && cp[0] == '.' && cp[1] == '.'))
        {

            id = ROOT;

        }

        else if (cl != 1 || cp[0] != '.')
        {

            unsigned int found = 0;

            for (i = 0; i < MEMSRV_FILES && id == ROOT; i++)
            {

                if (files[i].linked && files[i].length == cl && buffer_match(files[i].name, cp, cl))
                    found = getid(&files[i]);

            }

            id = found;

        }

        if (!id)
            return 0;

        offset += cl + 1;

    }

    return id;

}

static unsigned int stat(unsigned int id, struct record *record)
{

    struct file *file = getfile(id);

    if (id == ROOT)
    {

        record_init(record, ROOT, RECORD_TYPE_DIRECTORY, 0, 0, 1, ".");

        return sizeof (struct record);

    }

    if (file)
    {

        record_init(record, id, RECORD_TYPE_NORMAL, file->size, 0, file->length, file->name);

        return sizeof (struct record);

    }

    return 0;

}

static unsigned int readrecords(struct record *records, unsigned int count, unsigned int offset)
{

    unsigned int index = 0;
    unsigned int n = 0;
    unsigned int i;

    for (i = 0; i < MEMSRV_FILES + 2; i++)
    {

        struct record *record = &records[n / sizeof (struct record)];

        if (i > 1 && !files[i - 2].linked)
            continue;

        if (index++ < offset)
            continue;

        if (n + sizeof (struct record) > count)
            break;

        if (i == 0)
            record_init(record, ROOT, RECORD_TYPE_DIRECTORY, 0, 0, 1, ".");
        else if (i == 1)
            record_init(record, ROOT, RECORD_TYPE_DIRECTORY, 0, 0, 2, "..");
        else
            stat(getid(&files[i - 2]), record);

        record->offset = index;
        n += sizeof (struct record);

    }

    return n;

}

static unsigned int create(unsigned int parent, char *name, unsigned int length)
{

    unsigned int i;

    if (parent != ROOT || !length || length > MEMSRV_NAMESIZE || buffer_findbyte(name, length, '/') < length || walk(ROOT, name, length))
        return 0;

    for (i = 0; i < MEMSRV_FILES; i++)
    {

        struct file *file = &files[i];

        if (!file->used)
        {

            file->used = 1;
            file->linked = 1;
            file->tasks = 0;
            file->size = 0;
            file->cachepage = NONE;
            file->length = length;

            buffer_copy(file->name, name, length);

            return getid(file);

        }

    }

    return 0;

}

static unsigned int remove(unsigned int parent, unsigned int id)
{

    struct file *file = getfile(id);

    if (parent != ROOT || !file)
        return 0;

    file->linked = 0;

    release(file);

    return 1;

}

static unsigned long map(unsigned int source, unsigned int id, unsigned int size)
{

    struct file *file = getfile(id);
    struct mailbox *mailbox = getmailbox(source);
    unsigned int itask = (mailbox) ? mailbox->itask : 0;
    struct task *task = (mailbox) ? pool_gettask(itask) : 0;
    struct mmap_header *header;
    struct mmap_entry *entry;
    unsigned long vaddress;

    if (!file || !task)
        return 0;

    if (!size)
        size = file->size;

    if (!size || size > MEMSRV_FILESIZE)
        return 0;

    size = countpages(size) * MEMSRV_PAGESIZE;
    header = (struct mmap_header *)task->mmap;
    entry = findmapping(header, id);

    if (entry)
        return (entry->size >= size) ? entry->vaddress : 0;

    vaddress = findgap(header, size);

    if (!vaddress || !mmap_allocate(header, MMAP_TYPE_MEM, id, vaddress, size, MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE))
        return 0;

    file->tasks |= 1 << itask;

    return vaddress;

}

static unsigned int unmap(unsigned int source, unsigned int id)
{

    struct mailbox *mailbox = getmailbox(source);
    unsigned int itask = (mailbox) ? mailbox->itask : 0;
    struct task *task = (mailbox) ? pool_gettask(itask) : 0;
    unsigned int i;

    for (i = 0; i < MEMSRV_FILES; i++)
    {

        struct file *file = &files[i];

        if (file->used && getid(file) == id && task && (file->tasks & (1 << itask)))
        {

            struct mmap_header *header = (struct mmap_header *)task->mmap;
            struct mmap_entry *entry = findmapping(header, id);

            if (entry)
            {

                unmapcallback(entry->vaddress, entry->size);
                mmap_remove(header, entry);

            }

            file->tasks &= ~(1 << itask);

            release(file);

            return 1;

        }

    }

    return 0;

}

static unsigned int oncreaterequest(unsigned int source, unsigned int count, void *data)
{

    struct event_createrequest *request = data;
    struct event_createresponse response;

    spinlock_acquire(&spinlock);

    response.id = create(request->parent, (char *)(request + 1), request->count);

    spinlock_release(&spinlock);

    return kernel_place(inode, source, EVENT_CREATERESPONSE, sizeof (struct event_createresponse), &response);

}

static unsigned int onmmaprequest(unsigned int source, unsigned int count, void *data)
{

    struct event_mmaprequest *request = data;
    struct event_mmapresponse response;

    spinlock_acquire(&spinlock);

    response.address = map(source, request->id, request->size);

    spinlock_release(&spinlock);

    return kernel_place(inode, source, EVENT_MMAPRESPONSE, sizeof (struct event_mmapresponse), &response);

}

static unsigned int onmunmaprequest(unsigned int source, unsigned int count, void *data)
{

    struct event_munmaprequest *request = data;
    struct event_munmapresponse response;

    spinlock_acquire(&spinlock);

    response.status = unmap(source, request->id);

    spinlock_release(&spinlock);

    return kernel_place(inode, source, EVENT_MUNMAPRESPONSE, sizeof (struct event_munmapresponse), &response);

}

static unsigned int onreadrequest(unsigned int source, unsigned int count, void *data)
{

    unsigned char buffer[MESSAGE_SIZE];
    struct event_readrequest *request = data;
    struct event_readresponse *response = (struct event_readresponse *)buffer;
    struct file *file;

    if (request->count >= MESSAGE_SIZE - sizeof (struct event_readresponse))
        request->count = MESSAGE_SIZE - sizeof (struct event_readresponse);

    spinlock_acquire(&spinlock);

    file = getfile(request->id);

    if (request->id == ROOT)
        response->count = readrecords((struct record *)(response + 1), request->count, request->offset);
    else if (file)
        response->count = readdata(file, (unsigned char *)(response + 1), request->count, request->offset);
    else
        response->count = 0;

    spinlock_release(&spinlock);

    return kernel_place(inode, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + response->count, buffer);

}

static unsigned int onremoverequest(unsigned int source, unsigned int count, void *data)
{

    struct event_removerequest *request = data;
    struct event_removeresponse response;

    spinlock_acquire(&spinlock);

    response.status = remove(request->parent, request->id);

    spinlock_release(&spinlock);

    return kernel_place(inode, source, EVENT_REMOVERESPONSE, sizeof (struct event_removeresponse), &response);

}

static unsigned int onstatrequest(unsigned int source, unsigned int count, void *data)
{

    unsigned char buffer[MESSAGE_SIZE];
    struct event_statrequest *request = data;
    struct event_statresponse *response = (struct event_statresponse *)buffer;

    spinlock_acquire(&spinlock);

    response->count = stat(request->id, (struct record *)(response + 1));

    spinlock_release(&spinlock);

    return kernel_place(inode, source, EVENT_STATRESPONSE, sizeof (struct event_statresponse) + response->count, response);

}

static unsigned int onwalkrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_walkrequest *request = data;
    struct event_walkresponse response;

    spinlock_acquire(&spinlock);

    response.id = walk((request->parent) ? request->parent : ROOT, (char *)(request + 1), request->length);

    spinlock_release(&spinlock);

    return kernel_place(inode, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

}

static unsigned int onwriterequest(unsigned int source, unsigned int count, void *data)
{

    struct event_writerequest *request = data;
    struct event_writeresponse response;
    struct file *file;

    spinlock_acquire(&spinlock);

    file = getfile(request->id);
    response.count = (file) ? writedata(file, (unsigned char *)(request + 1), request->count, request->offset) : 0;

    spinlock_release(&spinlock);

    return kernel_place(inode, source, EVENT_WRITERESPONSE, sizeof (struct event_writeresponse), &response);

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_CREATEREQUEST:
        return oncreaterequest(source, count, data);

    case EVENT_MMAPREQUEST:
        return onmmaprequest(source, count, data);

    case EVENT_MUNMAPREQUEST:
        return onmunmaprequest(source, count, data);

    case EVENT_READREQUEST:
        return onreadrequest(source, count, data);

    case EVENT_REMOVEREQUEST:
        return onremoverequest(source, count, data);

    case EVENT_STATREQUEST:
        return onstatrequest(source, count, data);

    case EVENT_WALKREQUEST:
        return onwalkrequest(source, count, data);

    case EVENT_WRITEREQUEST:
        return onwriterequest(source, count, data);

    }

    return MESSAGE_FAILED;

}

unsigned long memsrv_getpage(unsigned int id, unsigned int offset)
{

    struct file *file;
    unsigned long paddress = 0;

    if (id <= ROOT || id - ROOT - 1 >= MEMSRV_FILES)
        return 0;

    file = &files[id - ROOT - 1];

    spinlock_acquire(&spinlock);

    if (file->used && offset < countpages(file->size) * MEMSRV_PAGESIZE)
        paddress = (unsigned long)getpage(file, offset - offset % MEMSRV_PAGESIZE);

    spinlock_release(&spinlock);

    return paddress;

}

void memsrv_untask(unsigned int itask)
{

    unsigned int i;

    spinlock_acquire(&spinlock);

    for (i = 0; i < MEMSRV_FILES; i++)
    {

        if (files[i].used && (files[i].tasks & (1 << itask)))
        {

            files[i].tasks &= ~(1 << itask);

            release(&files[i]);

        }

    }

    spinlock_release(&spinlock);

}

void memsrv_setup(unsigned long paddress, void (*unmap)(unsigned long vaddress, unsigned int size))
{

    node_operands_init(&operands, 0, operands_place, 0);
    service_init(&service);
    spinlock_init(&spinlock);

    base = paddress;

    for (nfree = 0; nfree < MEMSRV_PAGES; nfree++)
        next[nfree] = (nfree + 1 < MEMSRV_PAGES) ? nfree + 1 : NONE;

    freepages = 0;
    unmapcallback = unmap;
    inode = pool_picknode(0, &operands);

    if (inode)
        service_register(&service, inode, "mem");

}

