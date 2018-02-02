CC      = gcc
C_FLAGS = -Wall -g -pedantic -Werror
L_FLAGS = -lz -lpthread -lsqlite3

O_FILES = main.o socket.o io.o strings.o utils.o interpret.o help.o  \
	  action_safe.o mccp.o save.o event.o event-handler.o \
	  list.o stack.o crypt.o db.o

all: $(O_FILES)
	rm -f guildmud
	$(CC) -o guildmud $(O_FILES) $(L_FLAGS)

.c.o: all
	@$(CC) -c $(C_FLAGS) $<

clean:
	@echo Cleaning code $< ...
	@rm -f *.o
	@rm -f guildmud
	@rm -f *.*~
