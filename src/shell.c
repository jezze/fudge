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

void shell_command_cat(fs_node_t *fsnode)
{

	char buffer[256];

	uint32_t j;

	for (j = 0; j < read_fs(fsnode, 0, 256, buffer); j++)
		putc(buffer[j]);

	putc('\n');

}

void shell_command_clear()
{

	screen_clear();

}

void shell_command_help()
{

	puts("cat - Show content of test.txt file\n");
	puts("clear - Clear screen\n");
	puts("help - Show this dialog\n");
	puts("ls - List files and directories\n");

}

void shell_command_ls()
{

	int i = 0;
	struct dirent *node = 0;

	while ((node = readdir_fs(fs_root, i)) != 0)
	{

		fs_node_t *fsnode = finddir_fs(fs_root, node->name);

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

void shell_command_null()
{

	return;

}

void shell_init()
{

	screen_clear();

	puts("Fudge\n");
	puts("Copyright (c) 2009 Jens Nyberg\n\n");
	puts("Type `help` for a list of commands.\n\n");

	shell_buffer_clear();

	while(1)
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

				char *command = shell_buffer_read();

				if (strcmp(command, "") == 0)
					shell_command_null();

				else if (strcmp(command, "cat") == 0)
					shell_command_cat(finddir_fs(fs_root, "test.txt"));

				else if (strcmp(command, "clear") == 0)
					shell_command_clear();

				else if (strcmp(command, "help") == 0)
					shell_command_help();

				else if (strcmp(command, "ls") == 0)
					shell_command_ls();

				else
					puts("Command not found\n");

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
