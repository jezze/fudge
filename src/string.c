#include <system.h>

char *strcat(char *dest, const char *src)
{

	while (*dest != 0)
		*dest = *dest++;

	do
	{

		*dest++ = *src++;

	} while (*src != 0);

	return dest;

}

int strcmp(char *str1, char *str2)
{

	int i = 0;
	int failed = 0;

	while (str1[i] != '\0' && str2[i] != '\0')
	{

		if (str1[i] != str2[i])
		{

			failed = 1;
			break;

		}

		i++;

	}

	if ((str1[i] == '\0' && str2[i] != '\0') || (str1[i] != '\0' && str2[i] == '\0'))
		failed = 1;

	return failed;

}

char *strcpy(char *dest, const char *src)
{

	do
	{

		*dest++ = *src++;

	} while (*src != 0);

	return dest;

}

int strlen(const char *str)
{

	int32_t length;

	for (length = 0; *str != '\0'; str++)
		length++;

	return length;

}

