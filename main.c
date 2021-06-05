#include <stdio.h>
#include <string.h>
#include <signal.h>

char user_input[32768] = {0}, history[32768] = {0};
int cell[32768] = {0}, current = 0, err = 0, history_counter = 0;
FILE *file;

void input_parser(char *input, int cell[], int *current)
{
	for (int i = 0; i < 32768 && input[i] != '\n'; i++)
	{
		if (input[i] == '.')
			printf("%c", (cell[*current]));
		else if (input[i] == ',')
		{
			printf(", input: ");
			cell[*current] = getchar();
			while (getchar() != '\n')
				;
		}
		else if (input[i] == '[')
		{
			printf("Not yet implemented, idk how to do it...");
		}
		cell[*current] += (input[i] == '+') - (input[i] == '-') + ((255 - cell[*current]) * (cell[*current] < 0));
		*current += (input[i] == '>') - (input[i] == '<') * (*current > 0);
		history[history_counter] = input[i] * (input[i] == '+' || input[i] == '-' || input[i] == '>' || input[i] == '<' || input[i] == '.' || input[i] == ',' || input[i] == '[' || input[i] == ']');
		history_counter += input[i] == '+' || input[i] == '-' || input[i] == '>' || input[i] == '<' || input[i] == '.' || input[i] == ',' || input[i] == '[' || input[i] == ']';
	}
}

void ctrlc_handler()
{
	for (int i = 0; i < 32768; i++)
	{
		user_input[i] = 0;
		cell[i] = 0;
	}
	current = 0;
	err = 0;
}

int main(int argc, char **argv)
{
	signal(SIGINT, ctrlc_handler);

	if (argc > 1)
	{
		file = fopen(argv[1], "r");
		if (file == NULL)
		{
			err++;
			fprintf(stderr, "Error %i: file %s does not exist\n", err, argv[1]);
			return err;
		}
		fgets(user_input, sizeof(user_input), file);
		fclose(file);
		input_parser(user_input, cell, &current);
		printf("\n");
		return err;
	}

	printf(">>> ");
	while (fgets(user_input, 32768, stdin) && strcmp(user_input, "exit\n") != 0)
	{
		if (strcmp(user_input, "help\n") == 0)
		{
			printf("Commands:\n"
				   "	help	Shows this help message\n"
				   "	load	Loads file without exiting\n"
				   "	pac	Prints all cells\n"
				   "	pcp	Prints Current Pointer\n"
				   "	pcv	Prints Current cell Value\n"
				   "	exit	Exits the interpreter\n"
				   "Usage:\n"
				   "	%s <file>\n",
				   argv[0]);
		}
		else if (strcmp(user_input, "load\n") == 0)
		{
			printf("File name: ");
			fgets(user_input, 32768, stdin);
			user_input[strlen(user_input) - 1] = '\0';
			file = fopen(user_input, "r");
			if (file == NULL)
			{
				err++;
				fprintf(stderr, "Error %i: file %s does not exist", err, user_input);
			}
			else
			{
				fgets(user_input, sizeof(user_input), file);
				fclose(file);
				input_parser(user_input, cell, &current);
			}
		}
		else if (strcmp(user_input, "pac\n") == 0)
		{
			for (int i = 0; i < 32768; i++)
				if (cell[i] != 0)
					printf("%i	", i);
			printf("\n");
			for (int i = 0; i < 32768; i++)
				if (cell[i] != 0)
					printf("%i	", cell[i]);
		}
		else if (strcmp(user_input, "pcv\n") == 0)
			printf("%i", (cell[current]));
		else if (strcmp(user_input, "pcp\n") == 0)
			printf("%i", current);
		else if (strcmp(user_input, "phi\n") == 0)
			printf("%s", history);
		else
			input_parser(user_input, cell, &current);
		if (argc > 1)
		{
			printf("\n");
			return err;
		}
		printf("\n>>> ");
	}
	printf("\n");
	return err;
}