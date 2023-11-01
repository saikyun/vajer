CC=clang
CFLAGS=-g -D_THREAD_SAFE -Werror
INCLUDES=-Ilib -I/usr/local/include/SDL2 -I/usrc/local/include/SDL2_ttf
LIBS=-Llib/tinycc -ltcc -L/usr/local/lib -lSDL2 

test: FORCE
# experiment
#	./lib/tinycc/tcc -bt10 -DCOMPILED_WITH_TCC $(INCLUDES) -I. $(LIBS) $(CFLAGS) -run experiments/prototype.c
# standard one
	./lib/tinycc/tcc -bt10 -DCOMPILED_WITH_TCC $(INCLUDES) -I. $(LIBS) $(CFLAGS) -run test/test.c -Wall
#	./lib/tinycc/tcc $(INCLUDES) -I. $(LIBS) $(CFLAGS) test/test.c -o build/test

clang-test: clang-test-build
	./build/test

clang-test-build:
	clang $(CFLAGS) $(INCLUDES) -I. $(LIBS) -DSTACKTRACE_ON test/test.c -o build/test

clang-test-build2:
	clang $(CFLAGS) $(INCLUDES) -I. $(LIBS) -DSTACKTRACE_ON build/evaled.c -o build/test
#	./build/test

clean:
	rm -rf build/*

FORCE: