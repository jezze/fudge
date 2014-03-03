#include <fudge.h>
#include "token.h"
#include "parse.h"

static unsigned int open_path(unsigned int index, unsigned int indexw, unsigned int count, char *buffer)
{

    if (memory_match(buffer, "/", 1))
        return call_open(index, CALL_DR, count - 1, buffer + 1);

    return call_open(index, indexw, count, buffer);

}

static void execute_command(struct command *command, char *buffer)
{

    unsigned int i;

    if (!open_path(CALL_L1, CALL_L0, command->binary.count, buffer + command->binary.index))
        return;

    for (i = 0; i < command->ins; i++)
    {

        unsigned int index = CALL_I1 + i * 2;

        if (command->in[i].index.count)
            index = CALL_I1 + ascii_read_value(buffer + command->in[i].index.index, command->in[i].index.count, 10) * 2;

        if (command->in[i].path.count)
        {

            open_path(index, CALL_DW, command->in[i].path.count, buffer + command->in[i].path.index);

        }

        else if (command->in[i].data.count)
        {

            if (call_open(CALL_L6, CALL_DR, 17, "system/pipe/clone"))
            {

                if (call_open(CALL_L7, CALL_L6, 4, "../0"))
                {

                    call_write(CALL_L7, 0, command->in[i].data.count, buffer + command->in[i].data.index);
                    call_close(CALL_L7);
                    call_open(index, CALL_L6, 4, "../1");

                }

            }

        }

    }

    for (i = 0; i < command->outs; i++)
    {

        unsigned int index = CALL_O1 + i * 2;

        if (command->out[i].index.count)
            index = CALL_O1 + ascii_read_value(buffer + command->out[i].index.index, command->out[i].index.count, 10) * 2;

        if (command->out[i].path.count)
            open_path(index, CALL_DW, command->out[i].path.count, buffer + command->out[i].path.index);

    }

    call_spawn(CALL_L1);
    call_close(CALL_L1);

    for (i = 0; i < command->ins; i++)
    {

        unsigned int index = CALL_I1 + i * 2;

        if (command->in[i].index.count)
            index = CALL_I1 + ascii_read_value(buffer + command->in[i].index.index, command->in[i].index.count, 10) * 2;

        if (command->in[i].path.count)
        {

            call_close(index);

        }

        else if (command->in[i].data.count)
        {

            call_close(index);
            call_close(CALL_L6);

        }

    }

    for (i = 0; i < command->outs; i++)
    {

        unsigned int index = CALL_O1 + i * 2;

        if (command->out[i].index.count)
            index = CALL_O1 + ascii_read_value(buffer + command->out[i].index.index, command->out[i].index.count, 10) * 2;

        if (command->out[i].path.count)
            call_close(index);

    }

}

static void execute(struct token_state *state, struct expression *expression)
{

    unsigned int pindex;

    for (pindex = 0; pindex < expression->pipes; pindex++)
    {

        struct pipe *pipe = &expression->pipe[pindex];
        unsigned int cindex;

        for (cindex = 0; cindex < pipe->commands; cindex++)
            call_open(CALL_L2 + cindex, CALL_DR, 17, "system/pipe/clone");

        for (cindex = 0; cindex < pipe->commands; cindex++)
        {

            if (cindex != pipe->commands - 1)
                call_open(CALL_O1, CALL_L2 + cindex, 4, "../0");
            else
                call_open(CALL_O1, CALL_O0, 0, 0);

            execute_command(&pipe->command[cindex], state->buffer);
            call_open(CALL_I1, CALL_L2 + cindex, 4, "../1");

        }

        for (cindex = 0; cindex < pipe->commands; cindex++)
            call_close(CALL_L2 + cindex);

    }

}

void main()
{

    char buffer[FUDGE_BSIZE];
    struct token_state state;
    struct expression expression;

    if (!call_open(CALL_L0, CALL_DR, 4, "bin/"))
        return;

    memory_clear(&expression, sizeof (struct expression));
    token_init_state(&state, call_read(CALL_I0, 0, FUDGE_BSIZE, buffer), buffer);

    if (parse(&state, &expression))
        execute(&state, &expression);

    call_close(CALL_L0);

}

