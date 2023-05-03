CC=clang
CFLAGS=-g
INCLUDES=-Ilib
LIBS=-Llib/tinycc -ltcc

test: FORCE
	./lib/tinycc/tcc $(INCLUDES) -I. $(LIBS) $(CFLAGS) -run test/test.c
#	./lib/tinycc/tcc $(INCLUDES) -I. $(LIBS) $(CFLAGS) test/test.c -o build/test

clang-test: clang-test-build
	./build/test

clang-test-build:
	clang $(CFLAGS) $(INCLUDES) -I. $(LIBS) -DSTACKTRACE_ON test/test.c -o build/test

clean:
	rm -rf build/*

FORCE: