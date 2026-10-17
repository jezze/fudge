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

struct file
{

    unsigned int used;
    unsigned int linked;
    unsigned int tasks;
    unsigned int size;
    unsigned int length;
    char name[MEMSRV_NAMESIZE];

};

static struct node_operands operands;
static struct service service;
static struct spinlock spinlock;
static struct file files[MEMSRV_FILES];
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

static unsigned char *getdata(struct file *file)
{

    return (unsigned char *)(base + (file - files) * MEMSRV_FILESIZE);

}

/* every task sees a file at the same address */
static unsigned long getvaddress(struct file *file)
{

    return KERNEL_VMEM + (file - files) * MEMSRV_FILESIZE;

}

static struct task *gettask(unsigned int source, unsigned int *itask)
{

    struct resource *resource = pool_getnoderesource(source);

    if (resource && resource->type == RESOURCE_MAILBOX)
    {

        struct mailbox *mailbox = resource->data;

        *itask = mailbox->itask;

        return pool_gettask(mailbox->itask);

    }

    return 0;

}

static void release(struct file *file)
{

    if (!file->linked && !file->tasks)
        file->used = 0;

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

/* the directory reads as ".", ".." and then a record for each file; a record's offset is where the next read starts, as with cpio */
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
            file->length = length;

            buffer_copy(file->name, name, length);
            buffer_clear(getdata(file), MEMSRV_FILESIZE);

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

static unsigned long map(unsigned int source, unsigned int id)
{

    struct file *file = getfile(id);
    unsigned int itask;
    struct task *task = gettask(source, &itask);

    if (!file || !task)
        return 0;

    if (!(file->tasks & (1 << itask)))
    {

        if (!mmap_allocate((struct mmap_header *)task->mmap, MMAP_TYPE_NORMAL, (unsigned long)getdata(file), getvaddress(file), MEMSRV_FILESIZE, MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE))
            return 0;

        file->tasks |= 1 << itask;

    }

    return getvaddress(file);

}

static unsigned int unmap(unsigned int source, unsigned int id)
{

    unsigned int itask;
    struct task *task = gettask(source, &itask);
    unsigned int i;

    for (i = 0; i < MEMSRV_FILES; i++)
    {

        struct file *file = &files[i];

        if (file->used && getid(file) == id && task && (file->tasks & (1 << itask)))
        {

            struct mmap_header *header = (struct mmap_header *)task->mmap;
            struct mmap_entry *entry = mmap_find(header, getvaddress(file));

            if (entry)
                mmap_remove(header, entry);

            unmapcallback(getvaddress(file), MEMSRV_FILESIZE);

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

    response.address = map(source, request->id);

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
        response->count = buffer_read(response + 1, request->count, getdata(file), file->size, request->offset);
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
    response.count = (file) ? buffer_write(getdata(file), MEMSRV_FILESIZE, request + 1, request->count, request->offset) : 0;

    if (file && response.count && request->offset + response.count > file->size)
        file->size = request->offset + response.count;

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

    node_operands_init(&operands, 0, operands_place);
    service_init(&service);
    spinlock_init(&spinlock);

    base = paddress;
    unmapcallback = unmap;
    inode = pool_picknode(0, &operands);

    if (inode)
        service_register(&service, inode, "mem");

}

