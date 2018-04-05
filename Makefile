CC      = gcc
C_FLAGS = -Wall -pedantic -Werror -g
L_FLAGS = -lz -lpthread -lsqlite3
C_FLAGS_TEST = -Wall -pedantic -Werror -g -Isrc/
L_FLAGS_TEST = -lz -lpthread -lsqlite3 -lcheck -Lsrc/

SRC_FILES = $(wildcard src/*.c)
OBJ_FILES = $(filter-out src/main.o, $(SRC_FILES:.c=.o))
CHECK_FILES = $(wildcard tests/*.test)
CHECK_FILES_EXE = $(CHECK_FILES:.test=.run)

all: src/main.o src/libguildmud.a src/crypt_blowfish-1.3-mini/libblowfish-1.3.a
	@$(CC) -o src/guildmud src/main.o src/libguildmud.a src/crypt_blowfish-1.3-mini/libblowfish-1.3.a $(L_FLAGS)

src/libguildmud.a: $(OBJ_FILES)
	@ar ru $@ $^
	@ranlib $@

.c.o: all
	@$(CC) -c $(C_FLAGS) -o $@ $<


test: $(CHECK_FILES_EXE)
	@find tests/ -depth 1 -name '*.run' -exec {} \;
	

tests/%.run: tests/%.c src/libguildmud.a src/crypt_blowfish-1.3-mini/libblowfish-1.3.a
	@$(CC) -o $@ $< src/libguildmud.a src/crypt_blowfish-1.3-mini/libblowfish-1.3.a $(C_FLAGS_TEST) $(L_FLAGS_TEST) 

tests/%.c: tests/%.test
	@checkmk $< > $@

install:
	@echo To be implemented shortly...

.PHONY: clean
clean:
	@echo Cleaning code ...
	@rm -rf src/*.o src/guildmud src/libguildmud.a src/*~ tests/*.c tests/run* tests/*.dSYM src/*.dSYM
	@rm -rf src/crypt_blowfish-1.3-mini/*.o src/crypt_blowfish-1.3-mini/*~ src/crypt_blowfish-1.3-mini/*a src/crypt_blowfish-1.3-mini/mini-test

# Blowfish compilation rules

AS = $(CC)
LD = $(CC)
BFISH_CFLAGS = -g -W -Wall -Wbad-function-cast -Wcast-align -Wcast-qual -Wmissing-prototypes -Wstrict-prototypes -Wshadow -Wundef -Wpointer-arith -O2 -fomit-frame-pointer -funroll-loops
BFISH_ASFLAGS = -c
BFISH_LDFLAGS = -sL

CRYPT_OBJS = src/crypt_blowfish-1.3-mini/crypt_blowfish.o \
			 src/crypt_blowfish-1.3-mini/crypt_gensalt.o \
			 src/crypt_blowfish-1.3-mini/wrapper.o

crypt: src/crypt_blowfish-1.3-mini/libblowfish-1.3.a src/crypt_blowfish-1.3-mini/mini-test

src/crypt_blowfish-1.3-mini/libblowfish-1.3.a: $(CRYPT_OBJS)
	@ar ru $@ $^
	@ranlib $@

src/crypt_blowfish-1.3-mini/mini-test: src/crypt_blowfish-1.3-mini/mini-test.o src/crypt_blowfish-1.3-mini/libblowfish-1.3.a
	$(CC) -o $@ $< $(L_FLAGS) src/crypt_blowfish-1.3-mini/libblowfish-1.3.a

src/crypt_blowfish-1.3-mini/crypt_blowfish.o: src/crypt_blowfish-1.3-mini/crypt_blowfish.h
src/crypt_blowfish-1.3-mini/crypt_gensalt.o: src/crypt_blowfish-1.3-mini/crypt_gensalt.h
src/crypt_blowfish-1.3-mini/wrapper.o: src/crypt_blowfish-1.3-mini/crypt.h src/crypt_blowfish-1.3-mini/ow-crypt.h \
										 src/crypt_blowfish-1.3-mini/crypt_blowfish.h src/crypt_blowfish-1.3-mini/crypt_gensalt.h


