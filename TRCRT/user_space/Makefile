# user_program/Makefile
CC = gcc
CFLAGS = -I$(PWD)/include -I$(PWD)/../shared/include

# The final target should include tlb.o
trigger: trigger.o tlb.o cpuid_wrapper.o cores.o
	$(CC) -o trigger trigger.o tlb.o cpuid_wrapper.o cores.o

# Rule for compiling trigger.c
trigger.o: trigger.c
	$(CC) $(CFLAGS) -c trigger.c

# Rule for compiling tlb.c
tlb.o: ../shared/src/tlb.c
	$(CC) $(CFLAGS) -c ../shared/src/tlb.c

cpuid_wrapper.o: src/cpuid_wrapper.c
	$(CC) $(CFLAGS) -c src/cpuid_wrapper.c

cores.o: src/cores.c
	$(CC) $(CFLAGS) -c src/cores.c

# Clean rule
clean:
	rm -f trigger trigger.o tlb.o
