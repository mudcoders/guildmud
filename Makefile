CC      = gcc
C_FLAGS = -Wall -g -pedantic -Werror
L_FLAGS = -lz -lpthread -lsqlite3

SRC_FILES = $(wildcard src/*.c)
OBJ_FILES = $(filter-out src/main.o, $(SRC_FILES:.c=.o))


all: src/main.o src/libguildmud.a
	$(CC) -o src/guildmud src/main.o src/libguildmud.a $(L_FLAGS)

src/libguildmud.a: $(OBJ_FILES)
	ar ru $@ $^
	ranlib $@

.c.o: all
	$(CC) -c $(C_FLAGS) -o $@ $<

clean:
	@echo Cleaning code $< ...
	@rm -f src/*.o src/guildmud src/libguildmud.a src/*~

test:
	@echo To be implemented shortly...

install:
	@echo To be implemented shortly...
