all: build-cpp-asan build-cpp-msan

build-cpp-asan:
	AFL_USE_ASAN=1 afl-clang++ -O3 -s main.cpp -DEMIO_ENABLE_DEV_ASSERT -I../../../include -std=c++20 -L bin/ -o bin/fuzzy-asan

build-cpp-msan:
	AFL_USE_MSAN=1 afl-clang++ -O3 -s main.cpp -DEMIO_ENABLE_DEV_ASSERT -I../../../include -std=c++20 -L bin/ -o bin/fuzzy-msan

fuzzy-asan:
	LD_LIBRARY_PATH=bin AFL_BENCH_UNTIL_CRASH=1 afl-fuzz -i seeds/ -o output/ -D  -- ./bin/fuzzy-asan

fuzzy-msan:
	LD_LIBRARY_PATH=bin AFL_BENCH_UNTIL_CRASH=1 afl-fuzz -i seeds/ -o output-2/ -D  -- ./bin/fuzzy-msan
