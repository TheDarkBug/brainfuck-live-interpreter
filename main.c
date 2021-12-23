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
		} else if (input[i] == '-') {
			while (input[i++] == '-') minus_count++;
			fprintf(output, "%ssub byte [cell+rax], %i\n", indent, minus_count);
			minus_count = 0;
			i--;
		}
		if (input[i] == '.')
			fprintf(output, "%scall dot\n", indent);
		if (input[i] == ',')
			fprintf(output, "%scall comma\n", indent);
		else if (input[i] == '[') {
			fprintf(output, "%sloop%i:\n", indent, (*loop_counter)++);
			if (realloc(indent, 4 * (++indent_count)) == NULL) {
				fprintf(stderr, "Failed to realloc indent!\n");
				return 1;
			}
			strcat(indent, "    ");
		} else if (input[i] == ']') {
			fprintf(output, "%sjnz loop%i\n", indent, (*loop_counter) - 1);
			indent[(indent_count--) * 4] = '\0';
		} else if (input[i] == '>')
			fprintf(output, "%sinc rax\n", indent);
		else if (input[i] == '<')
			fprintf(output, "%sdec rax\n", indent);
	}
	fprintf(output, "    mov rdi, 0x0\n    call exit\ndot:\n    push rax\n    mov rdx, [cell+rax]\n    push rdx\n    mov rdi, 0x1\n    mov rsi, rsp\n    mov rdx, 0x1\n    mov rax, 0x1\n    syscall\n    pop rax\n    pop rax\n    ret\ncomma:\n    push rax\n    mov rdi, 0x0\n    mov rsi, readbuff\n    mov rdx, 0x1\n    mov rax, 0x0\n    syscall\n    pop rax\n    mov rdx, [readbuff]\n    mov [cell+rax], rdx\n    ret\nexit:\n    mov rax, 0x3c\n    syscall\nsection .data\ncell: times 32768 db 0x0\nreadbuff: db 0x0\n");
	fclose(output);
	return 0;
}

int cc(FILE* output, char* input) {
	int op_count = 0;
	fprintf(output, "#include <stdio.h>\n#include <unistd.h>\nint main(void){\nchar cells[32768]={0};\nint current=0;\n");
	for (int i = 0; i < 32768 && input[i] != '\n'; i++) {
		if (input[i] == '+' || input[i] == '-') {
			while (input[i] == '+' || input[i] == '-') {
				op_count++;
				i++;
			}
			fprintf(output, "cells[current]%c=%i;\n", input[i - 1], op_count);
			op_count = 0;
			i--;
		}
		if (input[i] == '.')
			fprintf(output, "printf(\"%%c\", cells[current]);\n");
		if (input[i] == ',')
			fprintf(output, "\n");
		else if (input[i] == '[')
			fprintf(output, "while(cells[current]){\n");
		else if (input[i] == ']')
			fprintf(output, "}\n");
		else if (input[i] == '>')
			fprintf(output, "current++;\n");
		else if (input[i] == '<')
			fprintf(output, "current--;\n");
	}
	fprintf(output, "return 0;\n}\n");
	fclose(output);
	return 0;
}

int compiler(char* fname, char* input, char lang, int* loop_counter) {
	char dir_name[] = "/tmp/tmp.bfli.XXXXXX";
	mkdtemp(dir_name);
	if (lang == 'a') {
		char* asm_fname = malloc(strlen(dir_name) + 10);
		sprintf(asm_fname, "%s/temp.asm", dir_name);
		FILE* temp_asmfp = fopen(asm_fname, "w");
		assembler(temp_asmfp, input, loop_counter);
		char command[1500] /* = malloc((strlen(dir_name) * 2) + (strlen(fname) * 3) + 35) */; // idk why but malloc here does not work
		sprintf(command, "nasm -f elf64 -o %s/%s.o %s; ld %s/%s.o -o %s", dir_name, fname, asm_fname, dir_name, fname, fname);
		system(command);
		// free(command);
	} else if (lang == 'c') {
		char* cfname = malloc(strlen(dir_name) + 8);
		sprintf(cfname, "%s/temp.c", dir_name);
		FILE* temp_cfile = fopen(cfname, "w");
		cc(temp_cfile, input);
		char* command = malloc(strlen(fname) + strlen(cfname) + 10);
		sprintf(command, "gcc -o %s %s", fname, cfname);
		system(command);
		free(command);
	}
	char* rmcmd = malloc(strlen(dir_name) + 9);
	sprintf(rmcmd, "rm -rf %s", dir_name);
	system(rmcmd);
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
	char user_input[32768] = {0}, history[32768] = {0}, *bin_file_name;
	int cell[32768] = {0}, current = 0, err = 0, history_counter = 0, loops[2][32768] = {0}, loop_counter = 0;
	int opt = 0;
	FILE *infile, *outfile;
	int opt_infile = 0, opt_outfile = 0, opt_compile = 0;
	char opt_lang = 'a'; // a for asm, c for c
	while ((opt = getopt(argc, argv, "i:o:c:l:")) != -1) {
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
		case 'c':
			opt_compile = 1;
			if (!optarg) {
				fprintf(stderr, "A file name is required\n");
				return 1;
			}
			bin_file_name = optarg;
			break;
		case 'l':
			if (strlen(optarg) > 1) {
				fprintf(stderr, "Lang option must be 'a' or 'c'.\n");
				return 1;
			}
			opt_lang = optarg[0];
			break;

		default:
			break;
		}
	}

	if (opt_infile || opt_outfile || opt_compile) {
		parse_file(infile, user_input);
		if (opt_infile && !opt_outfile && !opt_compile) {
			input_parser(user_input, cell, loops, history, &current, &loop_counter, &history_counter);
			printf("\n");
		} else if (opt_infile && (opt_outfile || opt_compile)) {
			if (opt_lang == 'a' && !opt_compile)
				assembler(outfile, user_input, &loop_counter);
			else if (opt_lang == 'c' && !opt_compile)
				cc(outfile, user_input);
			if (opt_compile)
				compiler(bin_file_name, user_input, opt_lang, &loop_counter);
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