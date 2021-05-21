#include <stdio.h>
#include <string.h>
#include <signal.h>

void ctrlc_handler() {}

int main(int argc, char **argv)
{
#ifdef DEBUG
	printf("%s launched with %i args", argv[0], argc - 1);
	if (argc > 1)
		printf(":\n");
	for (int i = 1; i < argc; i++)
		printf("%s ", argv[i]);
	printf("\n");
#endif
	signal(SIGINT, ctrlc_handler);
	char user_input[32768] = {0}, cmdbuf[128] = {0};
	int cell[32768] = {0}, current = 0, err = 0, loopbuf;
	while (1)
	{
		if (fgets(user_input, 32768, stdin) == NULL)
			break;
		sscanf(user_input, "%s", cmdbuf);
		for (int i = 0; i < 32768; i++)
		{
			if (user_input[i] == '\n')
				break;
			else if (strcmp(cmdbuf, "pcv") == 0)
			{
				printf("%i", (cell[current]));
				break;
			}
			else if (strcmp(cmdbuf, "pcp") == 0)
			{
				printf("%i", current);
				break;
			}
			if (user_input[i] == '.')
				printf("%c", (cell[current]));
			else if (user_input[i] == ',')
			{
				cell[current] = getchar();
				while (getchar() != '\n')
					;
			}
			else if (user_input[i] == '[')
			{
				printf("Not yet implemented, idk how to do it...");
			}
			cell[current] += (user_input[i] == '+') - (user_input[i] == '-') + ((255 - cell[current]) * (cell[current] < 0));
			current += (user_input[i] == '>') - (user_input[i] == '<') * (current > 0);
		}
		printf("\n");
	}
	fprintf(stderr, "Exit with %i errors\n", err);

	return err;
}