CC=clang
CFLAGS=-g
INCLUDES=-Ilib
LIBS=-Llib/tinycc -ltcc

test: FORCE
	./lib/tinycc/tcc $(INCLUDES) -I. $(LIBS) $(CFLAGS) -run test/test.c

test-clang:
	clang $(INCLUDES) -I. $(LIBS) test/test.c -o build/test
	./build/test

clean:
	rm -rf build/*

FORCE: