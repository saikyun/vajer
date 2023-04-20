CC=clang
CFLAGS=-g
INCLUDES=-Ilib
LIBS=-Llib/tinycc -ltcc
SOURCES=src/main.c src/lisp.h
TEST_SOURCES=test/test_parser.h test/test_tcc.h test/test.c

build/main: $(SOURCES)
	$(CC) $(INCLUDES) $(LIBS) $(CFLAGS) -o build/main src/main.c

run: FORCE
	tcc $(INCLUDES) $(LIBS) $(CFLAGS) -run src/main.c

test: FORCE
	tcc $(INCLUDES) -I. $(LIBS) $(CFLAGS) -run test/test.c

test-clang:
	clang $(INCLUDES) -I. $(LIBS) test/test.c -o build/test
	./build/test

clean:
	rm -rf build/*

FORCE: