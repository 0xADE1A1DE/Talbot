# user_program/Makefile
CC = gcc
CFLAGS = -I$(PWD)/include -I$(PWD)/../shared/include

# The final target should include tlb.o
trigger: trigger.o cores.o
	$(CC) -o trigger trigger.o cores.o

# Rule for compiling trigger.c
trigger.o: trigger.c
	$(CC) $(CFLAGS) -c trigger.c

cores.o: src/cores.c
	$(CC) $(CFLAGS) -c src/cores.c

# Clean rule
clean:
	rm -f trigger trigger.o
