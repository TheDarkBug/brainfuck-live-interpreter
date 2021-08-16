CC = cc
CFLAGS = -O3
FILES = main.c
LDFLAGS = 
NAME = "bfli"

build:
	$(CC) $(CFLAGS) -o $(NAME) $(FILES) $(LDFLAGS)

install: build
	cp $(NAME) /usr/bin/$(NAME)

run: build
	./$(NAME) test.bf

clean:
	rm $(NAME)

debug: CFLAGS = -g -Wall -Wextra -D DEBUG
debug: build
debug_run: debug run
debug_gdb: debug
debug_gdb:
	gdb ./$(NAME)