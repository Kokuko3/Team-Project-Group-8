builddir := build
buildlibdir := build/shared
srcdirs := src
libdirs := lib $(wildcard lib/*)

# inserted -I$(libdirs), this may need to be removed if it breaks things...
CC := gcc
CFLAGS := -Wall -std=gnu17 -pedantic -g
LFLAGS := -std=gnu17 -O3 -ggdb
LDLIBS := -lssl -lcrypto

.PHONY: clean all

all: clean main rx tx

clean:
	rm -f $(builddir)/*.o $(buildlibdir)/* main rx tx

#build: main.o rx.o tx.o lists.o local.o transfer.o
#	cc $(wildcard $(builddir)/*) -o main
#	cc $(wildcard $(builddir)/*) -o rx
#	cc $(wildcard $(builddir)/*) -o tx

main: main.o lists.o local.o transfer.o
	cc $(wildcard $(buildlibdir)/*) $(builddir)/main.o -o main $(LDLIBS)

rx: rx.o lists.o local.o transfer.o
	cc $(wildcard $(buildlibdir)/*) $(builddir)/rx.o -o rx $(LDLIBS)

tx: tx.o lists.o local.o transfer.o
	cc $(wildcard $(buildlibdir)/*) $(builddir)/tx.o -o tx $(LDLIBS)

VPATH = $(srcdirs) $(libdirs) $(builddir)
main.o: main.c lists.o local.o transfer.o
	cc -o $(builddir)/$@ -c $(CFLAGS) $<

rx.o: rx.c lists.o local.o transfer.o
	cc -o $(builddir)/$@ -c $(CFLAGS) $<

tx.o: tx.c lists.o local.o transfer.o
	cc -o $(builddir)/$@ -c $(CFLAGS) $<

lists.o: lists.c lists.h
	cc -o $(buildlibdir)/$@ -c $(CFLAGS) $<

local.o: local.c local.h
	cc -o $(buildlibdir)/$@ -c $(CFLAGS) $<

transfer.o: transfer.c transfer.h
	cc -o $(buildlibdir)/$@ -c $(CFLAGS) $<

