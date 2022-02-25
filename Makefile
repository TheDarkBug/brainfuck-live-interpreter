CC = cc
CFLAGS = -O2
FILES = main.c
LDFLAGS = 
NAME = bfli

ifeq ($(PLATFORM), wasm)
CC = emcc
CFLAGS += -DWASM --no-entry -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_FUNCTIONS="['_wasmain']"
NAME = bfli.wasm
endif

build:
	$(CC) $(CFLAGS) -o $(NAME) $(FILES) $(LDFLAGS)

install: build
	cp $(NAME) /usr/bin/$(NAME)

run: build
	./$(NAME) ./test.bf

clean:
	rm $(NAME)

debug: CFLAGS = -g -Wall -Wextra -D DEBUG -O0
debug: build
debug_run: debug run
debug_gdb: debug
debug_gdb:
	gdb ./$(NAME)