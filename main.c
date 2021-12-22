#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
			(*loop_counter)++;
			loops[0][*loop_counter] = *history_counter;
			loops[1][*loop_counter] = i;
		} else if (input[i] == ']') {
			if (cell[*current] > 0)
				i = loops[1][*loop_counter];
			else
				(*loop_counter)--;
		}
		cell[*current] += (input[i] == '+') - (input[i] == '-') + ((255 - cell[*current]) * (cell[*current] < 0));
		(*current) += (input[i] == '>') - (input[i] == '<') * ((*current) > 0);
		history[*history_counter] = input[i] * (input[i] == '+' || input[i] == '-' || input[i] == '>' || input[i] == '<' || input[i] == '.' || input[i] == ',' || input[i] == '[' || input[i] == ']');
		(*history_counter) += input[i] == '+' || input[i] == '-' || input[i] == '>' || input[i] == '<' || input[i] == '.' || input[i] == ',' || input[i] == '[' || input[i] == ']';
	}
}

int assembler(FILE* output, char* input, int* loop_counter) {
	int plus_count = 0, minus_count = 0, indent_count = 0;
	char* indent = malloc(5);
	sprintf(indent, "    ");
	fprintf(output, "section .text\nglobal _start\n_start:\n%sxor rax, rax\n", indent);
	for (int i = 0; i < 32768 && input[i] != '\n'; i++) {
		if (input[i] == '+') {
			while (input[i++] == '+') plus_count++;
			fprintf(output, "%sadd byte [cell+rax], %i\n", indent, plus_count);
			plus_count = 0;
			i--;
		}
		if (input[i] == '-') {
			while (input[i++] == '-') minus_count++;
			fprintf(output, "%ssub byte [cell+rax], %i\n", indent, minus_count);
			minus_count = 0;
			i--;
		}
		if (input[i] == '.') {
			while (input[i++] == '.') fprintf(output, "%scall dot\n", indent);
			i--;
		}
		if (input[i] == '[') {
			while (input[i++] == '[') {
				fprintf(output, "%sloop%i:\n", indent, (*loop_counter)++);
				if (realloc(indent, 4 * (++indent_count)) == NULL) {
					fprintf(stderr, "Failed to realloc indent!\n");
					return 1;
				}
				strcat(indent, "    ");
			}
			i--;
		}
		if (input[i] == ']') {
			while (input[i++] == ']') {
				fprintf(output, "%sjnz loop%i\n", indent, (*loop_counter) - 1);
				indent[(indent_count--) * 4] = '\0';
			}
			i--;
		}
		if (input[i] == '>') fprintf(output, "%sinc rax\n", indent);
		if (input[i] == '<') fprintf(output, "%sdec rax\n", indent);
	}
	fprintf(output, "    mov rdi, 0x0\n    call exit\ndot:\n    push rax\n    mov rdx, [cell+rax]\n    push rdx\n    mov rdi, 0x1\n    mov rsi, rsp\n    mov rdx, 0x1\n    mov rax, 0x1\n    syscall\n    pop rax\n    pop rax\n    ret\nexit:\n    mov rax, 0x3c\n    syscall\nsection .data\ncell: times 32768 db 0\n");
	fclose(output);
	return 0;
}

char* file_to_mem(FILE* source, size_t* size) {
	// FILE* source = fopen(filename, "r");
	char* target = NULL;
	if (size == NULL)
		size = malloc(sizeof(size_t));

	// get infile size
	fseek(source, 0, SEEK_END);
	(*size) = ftell(source);
	rewind(source);

	// actually read the infile
	target = malloc(((*size) + 1) * sizeof(*target));
	fread(target, (*size), 1, source);
	target[*size] = '\0';

	fclose(source);
	return target;
}

void parse_file(FILE* file, char* user_input) {
	size_t input_size = 0;

	strcpy(user_input, file_to_mem(file, &input_size));
	for (; input_size > 0; input_size--) {
		// if (user_input[input_size] == '\n') user_input[input_size] = ' ';
		int is_nl			   = (user_input[input_size] == '\n');
		user_input[input_size] = (user_input[input_size] * !is_nl) + (' ' * is_nl);
	}
}

int main(int argc, char** argv) {
	char user_input[32768] = {0}, history[32768] = {0};
	int cell[32768] = {0}, current = 0, err = 0, history_counter = 0, loops[2][32768] = {0}, loop_counter = 0;
	int opt = 0;
	FILE *infile, *outfile;
	int opt_infile = 0, opt_outfile = 0;
	while ((opt = getopt(argc, argv, "i:o:")) != -1) {
		switch (opt) {
		case 'i':
			opt_infile = 1;
			if (!optarg) {
				fprintf(stderr, "A file name is required\n");
				return 1;
			}
			infile = fopen(optarg, "r");
			if (infile == NULL) {
				fprintf(stderr, "Error %i: file %s does not exist\n", ++err, optarg);
				return err;
			}
			break;
		case 'o':
			opt_outfile = 1;
			if (!optarg) {
				fprintf(stderr, "A file name is required\n");
				return 1;
			}
			outfile = fopen(optarg, "w");
			if (outfile == NULL) {
				fprintf(stderr, "Error: cannot create %s: permission denied!\n", optarg);
				return 1;
			}
			break;

		default:
			break;
		}
	}

	if (opt_infile || opt_outfile) {
		parse_file(infile, user_input);
		if (opt_infile && !opt_outfile) {
			input_parser(user_input, cell, loops, history, &current, &loop_counter, &history_counter);
			printf("\n");
		} else if (opt_infile && opt_outfile) {
			assembler(outfile, user_input, &loop_counter);
		}
		return err;
	}

	printf("BFLI (BrainFuck Live Interpreter)\n"
		   "This program is under the GPL-3 License\n"
		   "https://thedarkbug.github.io/bfli.html\n"
		   "Type 'help' to get help.\n");

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
				   "	reset	Resets all cells and loop counter\n"
				   "	exit	Exits the interpreter\n"
				   "Usage:\n"
				   "	%s <option> <option arg>\n",
				   argv[0]);
		} else if (strcmp(user_input, "load\n") == 0) {
			printf("File name: ");
			fgets(user_input, 32768, stdin);
			user_input[strlen(user_input) - 1] = '\0';
			infile							   = fopen(user_input, "r");
			if (infile == NULL)
				fprintf(stderr, "Error %i: infile %s does not exist", ++err, user_input);
			else {
				fgets(user_input, sizeof(user_input), infile);
				fclose(infile);
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
		} else if (strcmp(user_input, "reset\n") == 0) {
			memset(user_input, 0, sizeof(user_input));
			memset(history, 0, sizeof(history));
			memset(cell, 0, sizeof(cell));
			memset(loops, 0, sizeof(loops));
			err				= 0;
			history_counter = 0;
			loop_counter	= 0;

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