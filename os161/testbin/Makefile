#
# Makefile for src/testbin (sources for programs installed in /testbin)
#

include ../defs.mk

all depend tags clean install:
	(cd add && $(MAKE) $@)
	(cd argtest && $(MAKE) $@)
	(cd badcall && $(MAKE) $@)
	(cd bigfile && $(MAKE) $@)
	(cd conman && $(MAKE) $@)
	(cd crash && $(MAKE) $@)
	(cd ctest && $(MAKE) $@)
	(cd dirconc && $(MAKE) $@)
	(cd dirseek && $(MAKE) $@)
	(cd dirtest && $(MAKE) $@)
	(cd f_test && $(MAKE) $@)
	(cd farm && $(MAKE) $@)
	(cd faulter && $(MAKE) $@)
	(cd filetest && $(MAKE) $@)
	(cd forkbomb && $(MAKE) $@)
	(cd fork && $(MAKE) $@)
	(cd forktest && $(MAKE) $@)
	(cd guzzle && $(MAKE) $@)
	(cd hash && $(MAKE) $@)
	(cd hog && $(MAKE) $@)
	(cd huge && $(MAKE) $@)
	(cd kitchen && $(MAKE) $@)
	(cd matmult && $(MAKE) $@)
	(cd multiply && $(MAKE) $@)
	(cd palin && $(MAKE) $@)
	(cd parallelvm && $(MAKE) $@)
	(cd printf && $(MAKE) $@)
	(cd randcall && $(MAKE) $@)
	(cd rmdirtest && $(MAKE) $@)
	(cd rmtest && $(MAKE) $@)
	(cd sink && $(MAKE) $@)
	(cd sort && $(MAKE) $@)
	(cd sty && $(MAKE) $@)
	(cd tail && $(MAKE) $@)
	(cd tictac && $(MAKE) $@)
	(cd triplehuge && $(MAKE) $@)
	(cd triplemat && $(MAKE) $@)
	(cd triplesort && $(MAKE) $@)
	(cd malloctest && $(MAKE) $@)
	(cd forkexecbomb && $(MAKE) $@)
	(cd stacktest && $(MAKE) $@)
	(cd waitexit1 && $(MAKE) $@)
	(cd waitexit2 && $(MAKE) $@)
	(cd waitexit3 && $(MAKE) $@)
	(cd waitexit4 && $(MAKE) $@)
	(cd waitexit5 && $(MAKE) $@)

	(cd bigprog && $(MAKE) $@)
	(cd triplebigprog && $(MAKE) $@)
	(cd vectormult && $(MAKE) $@)

# But not:
#    malloctest     (no malloc/free until you write it)
#    userthreads    (no support in kernel API in base system)
