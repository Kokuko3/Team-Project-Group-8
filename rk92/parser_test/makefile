builddir := build
srcdirs := src
libdirs := lib $(wildcard lib/*)

CC := gcc
CFLAGS := -Wall -std=c99 -pedantic -g
LFLAGS := -std=c99 -O3 -ggdb

.PHONY: clean all

all: clean build

clean:
	rm -f $(builddir)/* main

build: main.o local.o transfer.o
	cc $(wildcard $(builddir)/*) -o main
#	cc -o $@ $(CFLAGS) $<


VPATH = $(srcdirs) $(libdirs) $(builddir)
main.o: main.c local.o transfer.o
	cc -o $(builddir)/$@ -c $(CFLAGS) $<

local.o: local.c local.h
	cc -o $(builddir)/$@ -c $(CFLAGS) $<

transfer.o: transfer.c transfer.h
	cc -o $(builddir)/$@ -c $(CFLAGS) $<

