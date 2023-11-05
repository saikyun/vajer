CC=clang
# -Wno-deprecated-declarations is there for `getsegbyname` on macos which is deprecated but works
CFLAGS=-g -D_THREAD_SAFE -Werror -Wno-deprecated-declarations
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
#   for running evaled.c
#	clang $(CFLAGS) $(INCLUDES) -I. $(LIBS) -DSTACKTRACE_ON build/evaled.c -o build/test
	clang $(CFLAGS) $(INCLUDES) -I. $(LIBS) -DSTACKTRACE_ON test/test.c -o build/test

clang-test-build2:
	clang $(CFLAGS) $(INCLUDES) -I. $(LIBS) -DSTACKTRACE_ON build/evaled.c -o build/test
#	./build/test

emcc:
	mkdir -p build/web
	source ../emsdk/emsdk_env.sh
	emcc -v build/evaled.c -o build/web/index.html -sALLOW_MEMORY_GROWTH -s USE_SDL=2
	emrun build/web/index.html

clean:
	rm -rf build/*

FORCE: