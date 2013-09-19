#include <fudge.h>
#include "token.h"
#include "parse.h"

static unsigned int open_path(unsigned int index, unsigned int count, char *buffer)
{

    return memory_match(buffer, "/", 1) ? call_open(index, CALL_DR, count - 1, buffer + 1) : call_open(index, CALL_DW, count, buffer);

}

static void open_pipe(unsigned int index, unsigned int index0, unsigned int index1)
{

    char buffer[FUDGE_BSIZE];
    char num[32];
    unsigned int offset;
    unsigned int count = call_read(index, 0, 32, num);

    offset = memory_write(buffer, FUDGE_BSIZE, "system/pipe/", 12, 0);
    offset += memory_write(buffer, FUDGE_BSIZE, num, count, offset);
    offset += memory_write(buffer, FUDGE_BSIZE, "/0", 2, offset);

    call_open(index0, CALL_DR, offset, buffer);

    offset = memory_write(buffer, FUDGE_BSIZE, "system/pipe/", 12, 0);
    offset += memory_write(buffer, FUDGE_BSIZE, num, count, offset);
    offset += memory_write(buffer, FUDGE_BSIZE, "/1", 2, offset);

    call_open(index1, CALL_DR, offset, buffer);

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
            call_open(CALL_P1 + cindex, CALL_DR, 17, "system/pipe/clone");

        for (cindex = 0; cindex < pipe->count; cindex++)
        {

            struct command *command = &pipe->commands[cindex];

            open_pipe(CALL_P1 + cindex, CALL_DO, 0);

            if (cindex == pipe->count - 1)
                call_open(CALL_DO, CALL_D2, 0, 0);

            if (command->in0.path.count)
            {

                open_path(CALL_DI, command->in0.path.count, state->buffer + command->in0.path.index);

            }

            else if (command->in0.data.count)
            {

                call_open(CALL_D6, CALL_DR, 17, "system/pipe/clone");
                open_pipe(CALL_D6, CALL_D0, CALL_DI);
                call_write(CALL_D0, 0, command->in0.data.count, state->buffer + command->in0.data.index);

            }

            if (command->in1.path.count)
            {

                open_path(CALL_DC, command->in1.path.count, state->buffer + command->in1.path.index);

            }

            else if (command->in1.data.count)
            {

                call_open(CALL_D7, CALL_DR, 17, "system/pipe/clone");
                open_pipe(CALL_D7, CALL_D0, CALL_DC);
                call_write(CALL_D0, 0, command->in1.data.count, state->buffer + command->in1.data.index);

            }

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
                call_close(CALL_D6);

            if (command->in1.path.count)
                call_close(CALL_DC);
            else if (command->in1.data.count)
                call_close(CALL_D7);

            if (command->out0.path.count)
                call_close(CALL_DO);

            open_pipe(CALL_P1 + cindex, 0, CALL_DI);

        }

        for (cindex = 0; cindex < pipe->count; cindex++)
            call_close(CALL_P1 + cindex);

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

