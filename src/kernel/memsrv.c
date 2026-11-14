#include <fudge.h>
#include "resource.h"
#include "mmap.h"
#include "service.h"
#include "mailbox.h"
#include "task.h"
#include "pool.h"
#include "server.h"
#include "kernel.h"
#include "memsrv.h"

#define ROOT                            1
#define FILEPAGES                       (MEMSRV_FILESIZE / MEMSRV_PAGESIZE)
#define NONE                            0xFFFF

struct file
{

    unsigned int used;
    unsigned int linked;
    unsigned int ntasks;
    unsigned int size;
    unsigned int first;
    unsigned int last;
    unsigned int cachen;
    unsigned int cachepage;
    unsigned int length;
    char name[MEMSRV_NAMESIZE];

};

static struct spinlock spinlock;
static struct server server;
static struct service service;
static struct file files[MEMSRV_FILES];
static unsigned short next[MEMSRV_PAGES];
static unsigned int freepages;
static unsigned int nfree;
static unsigned long base;
static void (*unmapcallback)(unsigned long vaddress, unsigned int size);

static struct file *getused(unsigned int id)
{

    return (id > ROOT && id - ROOT - 1 < MEMSRV_FILES && files[id - ROOT - 1].used) ? &files[id - ROOT - 1] : 0;

}

static struct file *getfile(unsigned int id)
{

    struct file *file = getused(id);

    return (file && file->linked) ? file : 0;

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

static void copydata(struct file *file, unsigned char *buffer, unsigned int count, unsigned int offset, unsigned int write)
{

    unsigned int done = 0;

    while (done < count)
    {

        unsigned char *page = getpage(file, offset + done);
        unsigned int chunk = MEMSRV_PAGESIZE - (offset + done) % MEMSRV_PAGESIZE;

        if (chunk > count - done)
            chunk = count - done;

        if (write)
            buffer_copy(page, buffer + done, chunk);
        else
            buffer_copy(buffer + done, page, chunk);

        done += chunk;

    }

}

static unsigned int readdata(struct file *file, unsigned char *buffer, unsigned int count, unsigned int offset)
{

    if (offset >= file->size)
        return 0;

    if (count > file->size - offset)
        count = file->size - offset;

    copydata(file, buffer, count, offset, 0);

    return count;

}

static unsigned int writedata(struct file *file, unsigned char *buffer, unsigned int count, unsigned int offset)
{

    if (!count || offset + count < offset)
        return 0;

    if (offset + count > file->size && !grow(file, offset + count))
        return 0;

    copydata(file, buffer, count, offset, 1);

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

static void release(struct file *file)
{

    if (file->used && !file->linked && !file->ntasks)
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

    if (parent == ROOT && length && length <= MEMSRV_NAMESIZE && buffer_findbyte(name, length, '/') >= length && !walk(ROOT, name, length))
    {

        unsigned int i;

        for (i = 0; i < MEMSRV_FILES; i++)
        {

            struct file *file = &files[i];

            if (!file->used)
            {

                file->used = 1;
                file->linked = 1;
                file->ntasks = 0;
                file->size = 0;
                file->cachepage = NONE;
                file->length = length;

                buffer_copy(file->name, name, length);

                return getid(file);

            }

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
    struct mailbox *mailbox = mailbox_get(source);
    unsigned int itask = (mailbox) ? mailbox->itask : 0;
    struct task *task = pool_gettask(itask);

    if (!file || !task)
        return 0;

    if (!size)
        size = file->size;

    if (size && size <= MEMSRV_FILESIZE)
    {

        struct mmap_header *header;
        struct mmap_entry *entry;
        unsigned long vaddress;

        if (size > file->size && !grow(file, size))
            return 0;

        size = countpages(size) * MEMSRV_PAGESIZE;
        header = (struct mmap_header *)task->mmap;
        entry = findmapping(header, id);

        if (entry)
            return (entry->size >= size) ? entry->vaddress : 0;

        vaddress = findgap(header, size);

        if (!vaddress || !mmap_allocate(header, MMAP_TYPE_MEM, id, vaddress, size, MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE))
            return 0;

        file->ntasks++;

        return vaddress;

    }

    return 0;

}

static unsigned int unmap(unsigned int source, unsigned int id)
{

    struct file *file = getused(id);
    struct mailbox *mailbox = mailbox_get(source);
    unsigned int itask = (mailbox) ? mailbox->itask : 0;
    struct task *task = pool_gettask(itask);
    struct mmap_entry *entry = (file && task) ? findmapping((struct mmap_header *)task->mmap, id) : 0;

    if (entry)
    {

        unmapcallback(entry->vaddress, entry->size);
        mmap_remove((struct mmap_header *)task->mmap, entry);

        file->ntasks--;

        release(file);

        return 1;

    }

    return 0;

}

static unsigned int oncreaterequest(unsigned int source, unsigned int count, void *data)
{

    struct message_createrequest *request = message_validate(MESSAGE_TYPE_CREATEREQUEST, data, count);
    struct message_createresponse response;

    if (request)
    {

        spinlock_acquire(&spinlock);

        response.id = create(request->parent, (char *)(request + 1), request->count);

        spinlock_release(&spinlock);

        return kernel_place(server.resource.inode, source, MESSAGE_TYPE_CREATERESPONSE, sizeof (struct message_createresponse), &response);

    }

    return MESSAGE_FAILED;

}

static unsigned int onmmaprequest(unsigned int source, unsigned int count, void *data)
{

    struct message_mmaprequest *request = message_validate(MESSAGE_TYPE_MMAPREQUEST, data, count);
    struct message_mmapresponse response;

    if (request)
    {

        spinlock_acquire(&spinlock);

        response.address = map(source, request->id, request->size);

        spinlock_release(&spinlock);

        return kernel_place(server.resource.inode, source, MESSAGE_TYPE_MMAPRESPONSE, sizeof (struct message_mmapresponse), &response);

    }

    return MESSAGE_FAILED;

}

static unsigned int onmunmaprequest(unsigned int source, unsigned int count, void *data)
{

    struct message_munmaprequest *request = message_validate(MESSAGE_TYPE_MUNMAPREQUEST, data, count);
    struct message_munmapresponse response;

    if (request)
    {

        spinlock_acquire(&spinlock);

        response.status = unmap(source, request->id);

        spinlock_release(&spinlock);

        return kernel_place(server.resource.inode, source, MESSAGE_TYPE_MUNMAPRESPONSE, sizeof (struct message_munmapresponse), &response);

    }

    return MESSAGE_FAILED;

}

static unsigned int onreadrequest(unsigned int source, unsigned int count, void *data)
{

    unsigned char buffer[MESSAGE_SIZE];
    struct message_readrequest *request = message_validate(MESSAGE_TYPE_READREQUEST, data, count);
    struct message_readresponse *response = (struct message_readresponse *)buffer;

    if (request)
    {

        unsigned int length = (request->count < MESSAGE_SIZE - sizeof (struct message_readresponse)) ? request->count : MESSAGE_SIZE - sizeof (struct message_readresponse);
        struct file *file;

        spinlock_acquire(&spinlock);

        file = getfile(request->id);

        if (request->id == ROOT)
            response->count = readrecords((struct record *)(response + 1), length, request->offset);
        else if (file)
            response->count = readdata(file, (unsigned char *)(response + 1), length, request->offset);
        else
            response->count = 0;

        spinlock_release(&spinlock);

        response->buffer = 0;

        return kernel_place(server.resource.inode, source, MESSAGE_TYPE_READRESPONSE, sizeof (struct message_readresponse) + response->count, buffer);

    }

    return MESSAGE_FAILED;

}

static unsigned int onremoverequest(unsigned int source, unsigned int count, void *data)
{

    struct message_removerequest *request = message_validate(MESSAGE_TYPE_REMOVEREQUEST, data, count);
    struct message_removeresponse response;

    if (request)
    {

        spinlock_acquire(&spinlock);

        response.status = remove(request->parent, request->id);

        spinlock_release(&spinlock);

        return kernel_place(server.resource.inode, source, MESSAGE_TYPE_REMOVERESPONSE, sizeof (struct message_removeresponse), &response);

    }

    return MESSAGE_FAILED;

}

static unsigned int onstatrequest(unsigned int source, unsigned int count, void *data)
{

    unsigned char buffer[MESSAGE_SIZE];
    struct message_statrequest *request = message_validate(MESSAGE_TYPE_STATREQUEST, data, count);
    struct message_statresponse *response = (struct message_statresponse *)buffer;

    if (request)
    {

        spinlock_acquire(&spinlock);

        response->count = stat(request->id, (struct record *)(response + 1));

        spinlock_release(&spinlock);

        return kernel_place(server.resource.inode, source, MESSAGE_TYPE_STATRESPONSE, sizeof (struct message_statresponse) + response->count, response);

    }

    return MESSAGE_FAILED;

}

static unsigned int onwalkrequest(unsigned int source, unsigned int count, void *data)
{

    struct message_walkrequest *request = message_validate(MESSAGE_TYPE_WALKREQUEST, data, count);
    struct message_walkresponse response;

    if (request)
    {

        spinlock_acquire(&spinlock);

        response.id = walk((request->parent) ? request->parent : ROOT, (char *)(request + 1), request->length);

        spinlock_release(&spinlock);

        return kernel_place(server.resource.inode, source, MESSAGE_TYPE_WALKRESPONSE, sizeof (struct message_walkresponse), &response);

    }

    return MESSAGE_FAILED;

}

static unsigned int onwriterequest(unsigned int source, unsigned int count, void *data)
{

    struct message_writerequest *request = message_validate(MESSAGE_TYPE_WRITEREQUEST, data, count);
    struct message_writeresponse response;

    if (request)
    {

        struct file *file;

        spinlock_acquire(&spinlock);

        file = getfile(request->id);
        response.count = (file) ? writedata(file, (unsigned char *)(request + 1), request->count, request->offset) : 0;

        spinlock_release(&spinlock);

        return kernel_place(server.resource.inode, source, MESSAGE_TYPE_WRITERESPONSE, sizeof (struct message_writeresponse), &response);

    }

    return MESSAGE_FAILED;

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case MESSAGE_TYPE_CREATEREQUEST:
        return oncreaterequest(source, count, data);

    case MESSAGE_TYPE_MMAPREQUEST:
        return onmmaprequest(source, count, data);

    case MESSAGE_TYPE_MUNMAPREQUEST:
        return onmunmaprequest(source, count, data);

    case MESSAGE_TYPE_READREQUEST:
        return onreadrequest(source, count, data);

    case MESSAGE_TYPE_REMOVEREQUEST:
        return onremoverequest(source, count, data);

    case MESSAGE_TYPE_STATREQUEST:
        return onstatrequest(source, count, data);

    case MESSAGE_TYPE_WALKREQUEST:
        return onwalkrequest(source, count, data);

    case MESSAGE_TYPE_WRITEREQUEST:
        return onwriterequest(source, count, data);

    }

    return MESSAGE_FAILED;

}

unsigned long memsrv_getpage(unsigned int id, unsigned int offset)
{

    struct file *file;
    unsigned long paddress = 0;

    spinlock_acquire(&spinlock);

    file = getused(id);

    if (file && offset < countpages(file->size) * MEMSRV_PAGESIZE)
        paddress = (unsigned long)getpage(file, offset - offset % MEMSRV_PAGESIZE);

    spinlock_release(&spinlock);

    return paddress;

}

unsigned long memsrv_getbase(unsigned int target, unsigned int id)
{

    return (target == server.resource.inode && getfile(id)) ? memsrv_getpage(id, 0) : 0;

}

unsigned int memsrv_read(unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    struct file *file;
    unsigned int done = 0;

    spinlock_acquire(&spinlock);

    file = getfile(id);

    if (file)
        done = readdata(file, buffer, count, offset);

    spinlock_release(&spinlock);

    return done;

}

void memsrv_untask(unsigned int itask)
{

    struct task *task = pool_gettask(itask);
    struct mmap_header *header = (task) ? (struct mmap_header *)task->mmap : 0;
    unsigned int i;

    if (!header)
        return;

    spinlock_acquire(&spinlock);

    for (i = 0; i < header->nentries; i++)
    {

        struct mmap_entry *entry = &header->entries[i];
        struct file *file = (entry->type == MMAP_TYPE_MEM) ? getused(entry->paddress) : 0;

        if (file && file->ntasks)
        {

            file->ntasks--;

            release(file);

        }

    }

    spinlock_release(&spinlock);

}

void memsrv_setup(unsigned long paddress, void (*unmap)(unsigned long vaddress, unsigned int size))
{

    spinlock_init(&spinlock);
    server_init(&server, 0, operands_place, 0);
    service_init(&service, "mem");
    service_register(&service, pool_attachnode(&server.resource));

    base = paddress;

    for (nfree = 0; nfree < MEMSRV_PAGES; nfree++)
        next[nfree] = (nfree + 1 < MEMSRV_PAGES) ? nfree + 1 : NONE;

    freepages = 0;
    unmapcallback = unmap;

}

