#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void input_parser(char* input, int cell[], int loops[2][32768], char* history, int* current, int* loop_counter, int* history_counter) {
	for (int i = 0; i < 32768 && input[i] != '\n'; i++) {
		if (input[i] == '.')
			printf("%c", (cell[*current]));
		else if (input[i] == ',') {
			printf(", input: ");
			cell[*current] = getchar();
			while (getchar() != '\n')
				;
		} else if (input[i] == '[') {
			// *loop_counter++;
			loops[0][*loop_counter] = *history_counter;
			loops[1][*loop_counter] = i;
		} else if (input[i] == ']') {
			if (cell[*current] > 0)
				i = loops[1][*loop_counter];
			else
				loop_counter--;
		}
		cell[*current] += (input[i] == '+') - (input[i] == '-') + ((255 - cell[*current]) * (cell[*current] < 0));
		*current += (input[i] == '>') - (input[i] == '<') * (*current > 0);
		history[*history_counter] = input[i] * (input[i] == '+' || input[i] == '-' || input[i] == '>' || input[i] == '<' || input[i] == '.' || input[i] == ',' || input[i] == '[' || input[i] == ']');
		history_counter += input[i] == '+' || input[i] == '-' || input[i] == '>' || input[i] == '<' || input[i] == '.' || input[i] == ',' || input[i] == '[' || input[i] == ']';
	}
}

char* file_to_mem(const char* filename, int* err) {
	FILE* source = fopen(filename, "r");
	char* target = NULL;
	size_t size	 = 0;

	if (source == NULL) {
		fprintf(stderr, "Error %i: file %s does not exist\n", ++*err, filename);
		exit(*err);
	}

	// get file size
	fseek(source, 0, SEEK_END);
	size = ftell(source);
	rewind(source);

	// actually read the file
	target = malloc((size + 1) * sizeof(*target));
	fread(target, size, 1, source);
	target[size] = '\0';

	fclose(source);
	return target;
}

void ctrlc_handler() { printf("\n>>> "); }

int main(int argc, char** argv) {
	char user_input[32768] = {0}, history[32768] = {0};
	int cell[32768] = {0}, current = 0, err = 0, history_counter = 0, loops[2][32768] = {0}, loop_counter = 0;
	FILE* file;
	printf("BFLI (BrainFuck Live Interpreter)\n"
		   "This program is under the GPL-3 License\n"
		   "https://thedarkbug.github.io/bfli.html\n"
		   "Type 'help' to get help.\n");
	signal(SIGINT, ctrlc_handler);

	if (argc > 1) {
		strcpy(user_input, file_to_mem(argv[1], &err));
		input_parser(user_input, cell, loops, history, &current, &loop_counter, &history_counter);
		printf("\n");
		return err;
	}

	printf(">>> ");
	while (fgets(user_input, 32768, stdin) && strcmp(user_input, "exit\n") != 0) {
		if (strcmp(user_input, "help\n") == 0) {
			printf("Commands:\n"
				   "	help	Shows this help message\n"
				   "	load	Loads file without exiting\n"
				   "	pac	Prints all cells\n"
				   "	plc	Prints Loop count\n"
				   "	pcp	Prints Current Pointer\n"
				   "	pcv	Prints Current cell Value\n"
				   "	pcs	Prints Current status (for debugging)\n"
				   "	exit	Exits the interpreter\n"
				   "Usage:\n"
				   "	%s <file>\n",
				   argv[0]);
		} else if (strcmp(user_input, "load\n") == 0) {
			printf("File name: ");
			fgets(user_input, 32768, stdin);
			user_input[strlen(user_input) - 1] = '\0';
			file							   = fopen(user_input, "r");
			if (file == NULL)
				fprintf(stderr, "Error %i: file %s does not exist", ++err, user_input);
			else {
				fgets(user_input, sizeof(user_input), file);
				fclose(file);
				input_parser(user_input, cell, loops, history, &current, &loop_counter, &history_counter);
			}
		} else if (strcmp(user_input, "pac\n") == 0) {
			for (int i = 0; i < 32768; i++)
				if (cell[i] != 0)
					printf("%i	", i);
			printf("\n");
			for (int i = 0; i < 32768; i++)
				if (cell[i] != 0)
					printf("%i	", cell[i]);
		} else if (strcmp(user_input, "plc\n") == 0)
			printf("%i", loop_counter);
		else if (strcmp(user_input, "pcv\n") == 0)
			printf("%i", (cell[current]));
		else if (strcmp(user_input, "pcp\n") == 0)
			printf("%i", current);
		else if (strcmp(user_input, "phi\n") == 0)
			printf("%s", history);
		else if (strcmp(user_input, "pcs\n") == 0) {
			FILE* status = fopen("/proc/self/status", "r");
			int signal	 = 0;
			size_t n, m;
			unsigned char buff[8192];
			do {
				n = fread(buff, 1, sizeof buff, status);
				if (n) {
					if (++signal > 30)
						for (long unsigned int i = 0; i < sizeof(buff); i++)
							buff[i] = -buff[i];
					m = fwrite(buff, 1, n, stdout);
				} else
					m = 0;
			} while ((n > 0) && (n == m));
		} else
			input_parser(user_input, cell, loops, history, &current, &loop_counter, &history_counter);
		if (argc > 1) {
			printf("\n");
			return err;
		}
		printf("\n>>> ");
	}
	printf("\n");
	return err;
}