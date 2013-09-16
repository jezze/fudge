#include <fudge.h>
#include "token.h"
#include "parse.h"

static unsigned int open_path(unsigned int index, unsigned int count, char *buffer)
{

    return memory_match(buffer, "/", 1) ? call_open(index, CALL_DR, count - 1, buffer + 1) : call_open(index, CALL_DW, count, buffer);

}

static void open_temp(unsigned int index, unsigned int index2, unsigned int count, void *buffer)
{

    char buffer0[FUDGE_BSIZE];
    char buffer1[FUDGE_BSIZE];
    char num[32];
    unsigned int ccount, offset0 = 0, offset1 = 0;

    call_open(index2, CALL_DR, 17, "system/pipe/clone");
    ccount = call_read(index2, 0, 32, num);

    offset0 += memory_write(buffer0, FUDGE_BSIZE, "system/pipe/", 12, offset0);
    offset0 += memory_write(buffer0, FUDGE_BSIZE, num, ccount, offset0);
    offset0 += memory_write(buffer0, FUDGE_BSIZE, "/0", 2, offset0);
    offset1 += memory_write(buffer1, FUDGE_BSIZE, "system/pipe/", 12, offset1);
    offset1 += memory_write(buffer1, FUDGE_BSIZE, num, ccount, offset1);
    offset1 += memory_write(buffer1, FUDGE_BSIZE, "/1", 2, offset1);

    call_open(CALL_D0, CALL_DR, offset0, buffer0);
    call_write(CALL_D0, 0, count, buffer);
    call_close(CALL_D0);
    call_open(index, CALL_DR, offset1, buffer1);

}

static void close_temp(unsigned int index2)
{

    call_close(index2);

}

static void open_pipe(unsigned int index, unsigned int total)
{

    char buffer0[FUDGE_BSIZE];
    char buffer1[FUDGE_BSIZE];
    char num[32];
    unsigned int ccount, offset0 = 0, offset1 = 0;

    if (total < 2)
        return;

    call_open(CALL_P0, CALL_DR, 17, "system/pipe/clone");
    ccount = call_read(CALL_P0, 0, 32, num);

    offset0 += memory_write(buffer0, FUDGE_BSIZE, "system/pipe/", 12, offset0);
    offset0 += memory_write(buffer0, FUDGE_BSIZE, num, ccount, offset0);
    offset0 += memory_write(buffer0, FUDGE_BSIZE, "/0", 2, offset0);
    offset1 += memory_write(buffer1, FUDGE_BSIZE, "system/pipe/", 12, offset1);
    offset1 += memory_write(buffer1, FUDGE_BSIZE, num, ccount, offset1);
    offset1 += memory_write(buffer1, FUDGE_BSIZE, "/1", 2, offset1);

    call_open(CALL_DO, CALL_DR, offset0, buffer0);
    call_open(CALL_D5, CALL_DR, offset1, buffer1);

}

static void close_pipe(unsigned int index, unsigned int total)
{

    if (total < 2)
        return;

    call_open(CALL_DI, CALL_D5, 0, 0);

    if (index > 0)
        call_close(CALL_P1);

    if (index == total - 1)
        call_close(CALL_P0);
    else
        call_open(CALL_P1, CALL_P0, 0, 0);

}

static void execute(struct token_state *state, struct expression *expression)
{

    unsigned int pindex;
    unsigned int cindex;

    call_open(CALL_D1, CALL_DI, 0, 0);
    call_open(CALL_D2, CALL_DO, 0, 0);
    call_open(CALL_D3, CALL_DC, 0, 0);
    call_open(CALL_D4, CALL_DW, 0, 0);

    for (pindex = 0; pindex < expression->count; pindex++)
    {

        struct pipe *pipe = &expression->pipes[pindex];

        for (cindex = 0; cindex < pipe->count; cindex++)
        {

            struct command *command = &pipe->commands[cindex];

            open_pipe(cindex, pipe->count);

            if (cindex == 0)
                call_open(CALL_DI, CALL_D1, 0, 0);

            if (cindex == pipe->count - 1)
                call_open(CALL_DO, CALL_D2, 0, 0);

            if (command->in0.path.count)
                open_path(CALL_DI, command->in0.path.count, state->buffer + command->in0.path.index);
            else if (command->in0.data.count)
                open_temp(CALL_DI, CALL_D8, command->in0.data.count, state->buffer + command->in0.data.index);

            if (command->in1.path.count)
                open_path(CALL_DC, command->in1.path.count, state->buffer + command->in1.path.index);
            else if (command->in1.data.count)
                open_temp(CALL_DC, CALL_D9, command->in1.data.count, state->buffer + command->in1.data.index);

            if (command->out0.path.count)
                open_path(CALL_DO, command->out0.path.count, state->buffer + command->out0.path.index);

            call_open(CALL_DW, CALL_DR, 4, "bin/");
            open_path(CALL_D0, command->binary.count, state->buffer + command->binary.index);
            call_open(CALL_DW, CALL_D4, 0, 0);
            call_spawn(CALL_D0);
            call_close(CALL_D0);

            if (command->in0.path.count)
                call_close(CALL_DI);
            else if (command->in0.data.count)
                close_temp(CALL_D8);

            if (command->in1.path.count)
                call_close(CALL_DC);
            else if (command->in1.data.count)
                close_temp(CALL_D9);

            close_pipe(cindex, pipe->count);

        }

    }

    call_close(CALL_D1);
    call_close(CALL_D2);
    call_close(CALL_D3);
    call_close(CALL_D4);

}

void main()
{

    char buffer[FUDGE_BSIZE];
    struct token_state state;
    struct expression expression;

    memory_clear(&expression, sizeof (struct expression));
    token_init_state(&state, call_read(CALL_DI, 0, FUDGE_BSIZE, buffer), buffer);

    if (parse(&state, &expression))
        execute(&state, &expression);

}

