#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void interpretbuiltin(unsigned int count, char *command)
{

    if (memory_match(command, "exit", 4))
    {

        quit = 1;

    }

    if (memory_match(command, "cd ", 3))
    {

        command[count - 1] = '\0';

        if (file_walk(FILE_L8, command + 3))
        {

            file_duplicate(FILE_PW, FILE_L8);
            file_duplicate(FILE_CW, FILE_L8);

        }

    }

}

static void interpretslang(unsigned int count, char *command)
{

    char buffer[FUDGE_BSIZE];

    if (!file_walk(FILE_CP, "/bin/slang"))
        return;

    if (!file_walk(FILE_LA, "/system/buf:2"))
        return;

    if (!file_walk(FILE_LB, "/system/buf:3"))
        return;

    file_walkfrom(FILE_CI, FILE_LA, "idata");
    file_walkfrom(FILE_LC, FILE_LA, "odata");
    file_walkfrom(FILE_LD, FILE_LB, "idata");
    file_walkfrom(FILE_CO, FILE_LB, "odata");
    file_open(FILE_LC);
    call_spawn();
    file_writeall(FILE_LC, command, count);
    file_close(FILE_LC);
    file_open(FILE_LD);

    while ((count = file_read(FILE_LD, buffer, FUDGE_BSIZE)))
        file_writeall(FILE_PO, buffer, count);

    file_close(FILE_LD);

}

static void interpret(struct ring *ring)
{

    char command[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, command, FUDGE_BSIZE);

    if (count < 2)
        return;

    interpretbuiltin(count, command);
    interpretslang(count, command);

}

static void complete(struct ring *ring)
{

}

static void handle(struct ring *ring, unsigned char c)
{

    switch (c)
    {

    case '\0':
        break;

    case '\t':
        complete(ring);

        break;

    case '\b':
    case 0x7F:
        if (!ring_skipreverse(ring, 1))
            break;

        file_writeall(FILE_PO, "\b \b", 3);

        break;

    case '\r':
        c = '\n';

    case '\n':
        file_writeall(FILE_PO, &c, 1);
        ring_write(ring, &c, 1);
        interpret(ring);
        file_writeall(FILE_PO, "$ ", 2);

        break;

    default:
        ring_write(ring, &c, 1);
        file_writeall(FILE_PO, &c, 1);

        break;

    }

}

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned char inputbuffer[FUDGE_BSIZE];
    struct ring input;

    ring_init(&input, FUDGE_BSIZE, inputbuffer);
    file_open(FILE_PI);
    file_open(FILE_PO);
    file_writeall(FILE_PO, "$ ", 2);

    while (!quit && (count = file_read(FILE_PI, buffer, FUDGE_BSIZE)))
    {

        unsigned int i;

        for (i = 0; i < count; i++)
            handle(&input, buffer[i]);

    }

    file_close(FILE_PO);
    file_close(FILE_PI);

}

