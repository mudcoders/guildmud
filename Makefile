CC      = gcc
C_FLAGS = -Wall -pedantic -Werror
L_FLAGS = -lz -lpthread -lsqlite3
C_FLAGS_TEST = -Wall -pedantic -Werror -Isrc/
L_FLAGS_TEST = -lz -lpthread -lsqlite3 -lcheck -Lsrc/

SRC_FILES = $(wildcard src/*.c)
OBJ_FILES = $(filter-out src/main.o, $(SRC_FILES:.c=.o))
CHECK_FILES = $(wildcard tests/*.test)
CHECK_FILES_EXE = $(CHECK_FILES:.test=.run)

all: src/main.o src/libguildmud.a
	@$(CC) -o src/guildmud src/main.o src/libguildmud.a $(L_FLAGS)

src/libguildmud.a: $(OBJ_FILES)
	@ar ru $@ $^
	@ranlib $@

.c.o: all
	@$(CC) -c $(C_FLAGS) -o $@ $<


test: $(CHECK_FILES_EXE)
	@echo Compiling tests...
	

tests/%.run: tests/%.c src/libguildmud.a
	@$(CC) -o $@ $< src/libguildmud.a $(C_FLAGS_TEST) $(L_FLAGS_TEST) 
	# $@ -- Uncomment if we want to run the compiled test

tests/%.c: tests/%.test
	@checkmk $< > $@

install:
	@echo To be implemented shortly...

.PHONY: clean
clean:
	@echo Cleaning code ...
	@rm -rf src/*.o src/guildmud src/libguildmud.a src/*~ tests/*.c tests/run* tests/*.dSYM src/*.dSYM
