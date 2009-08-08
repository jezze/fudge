#include <system.h>
#include <screen.h>
#include <kbd.h>
#include <fs.h>
#include <shell.h>

char shellBuffer[SHELL_BUFFER_SIZE];
uint16_t shellBufferSize = 0;

void shell_buffer_clear()
{

	puts("fudge:/$ ");
	shellBufferSize = 0;

}

void shell_buffer_push(int8_t c)
{

	shellBuffer[shellBufferSize] = c;

	if (shellBufferSize < SHELL_BUFFER_SIZE)
		shellBufferSize++;


}

void shell_buffer_pop()
{

	if (shellBufferSize)
		shellBufferSize--;

}

char *shell_buffer_read()
{

	shell_buffer_push('\0');

	return shellBuffer;

}

uint16_t shell_buffer_size()
{

	return shellBufferSize;

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
		shell_command_cat(argc, (char **)argv);

	else if (strcmp(argv[0], "clear") == 0)
		shell_command_clear(argc, argv);

	else if (strcmp(argv[0], "help") == 0)
		shell_command_help(argc, argv);

	else if (strcmp(argv[0], "ls") == 0)
		shell_command_ls(argc, argv);

	else
	{

		puts(argv[0]);
		puts(": ");
		puts("Command not found\n");

	}

}

void shell_init()
{

	screen_clear();

	puts("Fudge\n");
	puts("Copyright (c) 2009 Jens Nyberg\n");
	puts("Type 'help' for a list of commands.\n\n");

	shell_buffer_clear();

	while (1)
	{

		if (kbd_buffer_size())
		{

			char c = kbd_buffer_read();

			if (c == '\b')
			{

				if (shell_buffer_size())
				{

					putc('\b');
					putc(' ');
					putc('\b');
					shell_buffer_pop();

				}

			}

			else if (c == '\n')
			{

				putc('\n');
				shell_buffer_push(c);

				char *command = shell_buffer_read();

				shell_interpret(command);
				shell_buffer_clear();

			}

			else
			{

				putc(c);
				shell_buffer_push(c);

			}

		}

	}

}
