
ARCH ?= i386

INCLUDES := ../../../include ../../../arch . ../../../build

CMACROS := ARCH="\"$(ARCH)\"" DEBUG MEM_TEST

CC = gcc

CFLAGS = -O -g -lrt
LDFLAGS = -O -g -lrt

ff: test.c test.h ../ff_simple.c
	@$(MAKE) -C ../../..          # need build/ARCH
	@$(CC) test.c -c $(CFLAGS) \
		$(foreach MACRO,$(CMACROS),-D $(MACRO)) -D FIRST_FIT
	@$(CC) ../ff_simple.c -c $(CFLAGS) \
		$(foreach INC,$(INCLUDES),-I$(INC)) \
		$(foreach MACRO,$(CMACROS),-D $(MACRO)) -D FIRST_FIT
	@$(CC) ff_simple.o test.o -o $@ $(LDFLAGS)

gma: test.c test.h ../gma.c
	@$(MAKE) -C ../../..          # need build/ARCH
	@$(CC) test.c -c $(CFLAGS) \
		$(foreach MACRO,$(CMACROS),-D $(MACRO)) -D GMA
	@$(CC) ../gma.c -c $(CFLAGS) \
		$(foreach INC,$(INCLUDES),-I$(INC)) \
		$(foreach MACRO,$(CMACROS),-D $(MACRO)) -D GMA
	@$(CC) gma.o test.o -o $@ $(LDFLAGS)

clean:
	-rm ff gma *.o
