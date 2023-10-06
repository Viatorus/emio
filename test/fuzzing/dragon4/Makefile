.DEFAULT_GOAL := build-all

export CC := afl-clang
export CXX := afl-clang++

build-all:
	$(MAKE) build-harden
	$(MAKE) build-asan
	$(MAKE) build-msan

.build-rust:
	cargo build --manifest-path rust_ref/Cargo.toml -Z unstable-options $(RUSTARGS) --out-dir build/lib/ --release

.build-cpp:
	cmake . -B build/$(TARGET) -DCMAKE_BUILD_TYPE=Release
	cmake --build build/$(TARGET)

.fuzzy:
	LD_LIBRARY_PATH=build/lib/ AFL_BENCH_UNTIL_CRASH=1 afl-fuzz -i seeds/ -o output/ -D  -- ./build/$(TARGET)/dragon4-fuzzer

build-harden: TARGET=harden
build-harden: export AFL_HARDEN=1
build-harden: .build-rust .build-cpp

build-asan: TARGET=asan
build-asan: export AFL_USE_ASAN=1
build-asan: .build-rust .build-cpp

build-msan: TARGET=msan
build-msan: export AFL_USE_MSAN=1
build-msan: export RUSTFLAGS=-Zsanitizer=memory -Zsanitizer-memory-track-origins
build-msan: export RUSTARGS=-Z build-std --target x86_64-unknown-linux-gnu
build-msan: .build-rust .build-cpp

fuzzy-msan: TARGET=msan
fuzzy-msan: .fuzzy

fuzzy-asan: TARGET=asan
fuzzy-asan: .fuzzy

fuzzy-harden: TARGET=harden
fuzzy-harden: .fuzzy

all-harden: build-harden fuzzy-harden
all-asan: build-asan fuzzy-asan
all-msan: build-msan fuzzy-msan
