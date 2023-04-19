all: build-rust build-cpp fuzzy

build-rust:
	cargo build --manifest-path rust_ref/Cargo.toml -Z unstable-options --out-dir bin/ --release

build-cpp:
	afl-clang++ -O3 -s main.cpp -I../../../include -std=c++20 -l rust_ref -L bin/ -o bin/fuzzy

fuzzy:
	LD_LIBRARY_PATH=bin AFL_BENCH_UNTIL_CRASH=1 afl-fuzz -i seeds/ -o output/ -D  -- ./bin/fuzzy
