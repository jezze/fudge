#include <fudge.h>
#include "token.h"
#include "parse.h"

static unsigned int open_path(unsigned int index, unsigned int indexw, unsigned int count, char *buffer)
{

    if (memory_match(buffer, "/", 1))
        return call_open(index, CALL_DR, count - 1, buffer + 1);

    return call_open(index, indexw, count, buffer);

}

static void execute_command(struct command *command, char *buffer, unsigned int pipe)
{

    if (call_open(CALL_L0, CALL_DR, 4, "bin/"))
    {

        if (open_path(CALL_L1, CALL_L0, command->binary.count, buffer + command->binary.index))
        {

            unsigned int i = 0;
            unsigned int o = 0;

            if (command->in.path.count)
                i = open_path(CALL_I1, CALL_DW, command->in.path.count, buffer + command->in.path.index);

            if (command->out.path.count)
                o = open_path(CALL_O1, CALL_DW, command->out.path.count, buffer + command->out.path.index);

            call_spawn(CALL_L1);
            call_close(CALL_L1);

            if (i)
                call_close(CALL_I1);

            if (o)
                call_close(CALL_O1);

        }

        call_close(CALL_L0);

    }

}

static void execute(struct token_state *state, struct expression *expression)
{

    unsigned int pindex;

    for (pindex = 0; pindex < expression->pipes; pindex++)
    {

        struct pipe *pipe = &expression->pipe[pindex];
        unsigned int clast = pipe->commands - 1;
        unsigned int cindex;

        for (cindex = 0; cindex < clast; cindex++)
        {

            call_open(CALL_L2 + cindex, CALL_DR, 17, "system/pipe/clone");
            call_open(CALL_O1, CALL_L2 + cindex, 4, "../0");
            execute_command(&pipe->command[cindex], state->buffer, CALL_L2 + cindex);
            call_open(CALL_I1, CALL_L2 + cindex, 4, "../1");
            call_close(CALL_L2 + cindex);

        }

        call_open(CALL_O1, CALL_O0, 0, 0);
        execute_command(&pipe->command[clast], state->buffer, CALL_L2);

    }

}

void main()
{

    char buffer[FUDGE_BSIZE];
    struct token_state state;
    struct expression expression;

    memory_clear(&expression, sizeof (struct expression));
    token_init_state(&state, call_read(CALL_I0, 0, FUDGE_BSIZE, buffer), buffer);

    if (parse(&state, &expression))
        execute(&state, &expression);

}

