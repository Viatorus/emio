all: build-cpp-asan build-cpp-msan

build-cpp-asan:
	AFL_USE_ASAN=1 AFL_USE_UBSAN=1 afl-clang++ -O3 -s main.cpp -I../../../include -std=c++20 -L bin/ -o bin/fuzzy

build-cpp-msan:
	AFL_USE_MSAN=1 AFL_USE_UBSAN=1 afl-clang++ -O3 -s main.cpp -I../../../include -std=c++20 -L bin/ -o bin/fuzzy

fuzzy-asan:
	LD_LIBRARY_PATH=bin AFL_BENCH_UNTIL_CRASH=1 afl-fuzz -i seeds/ -o output/ -D  -- ./bin/fuzzy

fuzzy-msan:
	LD_LIBRARY_PATH=bin AFL_BENCH_UNTIL_CRASH=1 afl-fuzz -i seeds/ -o output/ -D  -- ./bin/fuzzy
