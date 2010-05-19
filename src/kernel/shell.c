#include <system.h>
#include <screen.h>
#include <kbd.h>
#include <fs.h>
#include <shell.h>

char shellBuffer[SHELL_BUFFER_SIZE];
uint16_t shellBufferSize = 0;

void shell_buffer_clear()
{

	shellBufferSize = 0;

}

void shell_buffer_push(char c)
{

	if (shellBufferSize < SHELL_BUFFER_SIZE)
	{

		shellBuffer[shellBufferSize] = c;
		shellBufferSize++;

	}

}

char shell_buffer_pop()
{

	if (shellBufferSize)
	{

		shellBufferSize--;
		return shellBuffer[shellBufferSize];
	}

	return 0;

}

char *shell_buffer_read()
{

	shell_buffer_push('\0');

	return shellBuffer;

}

void shell_clear()
{

	puts("fudge:/$ ");
	shell_buffer_clear();

}

void shell_command_cat(int argc, char *argv[])
{

	if (argc > 1)
	{

		fs_node_t *fsnode = fs_directory_find(fsRoot, argv[1]);

		char buffer[256];

		uint32_t j;

		for (j = 0; j < fs_read(fsnode, 0, 256, buffer); j++)
			putc(buffer[j]);

		putc('\n');

	}

}

void shell_command_clear(int argc, char *argv[])
{

	screen_clear();

}

void shell_command_help(int argc, char *argv[])
{

	puts("cat - Show content of a file\n");
	puts("clear - Clear screen\n");
	puts("help - Show this dialog\n");
	puts("ls - List files and directories\n");

}

void shell_command_ls(int argc, char *argv[])
{

	int i = 0;
	directory_entry_t *node = 0;

	while ((node = fs_directory_read(fsRoot, i)) != 0)
	{

		fs_node_t *fsnode = fs_directory_find(fsRoot, node->name);

		if ((fsnode->flags & 0x7) == FS_DIRECTORY)
		{

			puts("/");
			puts(node->name);
			puts("\n");

		}

		else
		{

			puts(node->name);
			puts("\n");

		}

		i++;

	}

}

void shell_command_null(int argc, char *argv[])
{

	return;

}

void shell_interpret(char *command)
{

	int argc = 0;
	char *argv[32];

	int start = 0;
	int current = 0;

	int i;
	int j = 0;

	while (command[current] != '\0')
	{

		if (command[current] == ' ' || command[current] == '\n')
		{

			j = 0;

			for (i = start; i < current; i++)
			{

				argv[argc][j] = command[i];

				j++;

			}

			argv[argc][j] = '\0';
			argc++;

			start = current + 1;

		}

		current++;

	}

	if (strcmp(argv[0], "") == 0)
		shell_command_null(argc, argv);

	else if (strcmp(argv[0], "cat") == 0)
		shell_command_cat(argc, argv);

	else if (strcmp(argv[0], "clear") == 0)
		shell_command_clear(argc, argv);

	else if (strcmp(argv[0], "help") == 0)
		shell_command_help(argc, argv);

	else if (strcmp(argv[0], "ls") == 0)
		shell_command_ls(argc, argv);

	else
	{

		puts(argv[0]);
		puts(": Command not found\n");

	}

	shell_clear();

}

void shell_init()
{

	screen_clear();

	puts("Fudge\n");
	puts("Copyright (c) 2009 Jens Nyberg\n");
	puts("Type 'help' for a list of commands.\n\n");

	shell_clear();

	while (1)
	{

		char c;

		if ((c = kbd_buffer_read()))
		{

			if (c == '\b')
			{

				if (shell_buffer_pop())
				{

					putc('\b');
					putc(' ');
					putc('\b');

				}

			}

			else
			{

				shell_buffer_push(c);
				putc(c);

			}

			if (c == '\n')
				shell_interpret(shell_buffer_read());

		}

	}

}
