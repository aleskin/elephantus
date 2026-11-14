# Thin wrapper over CMake. Override the cmake binary with `make CMAKE=/path/to/cmake`.
CMAKE     ?= cmake
CTEST     ?= $(patsubst %cmake,%ctest,$(CMAKE))
BUILD_DIR ?= build
JOBS      ?= $(shell nproc 2>/dev/null || echo 4)

.PHONY: all build test run clean format

all: build

$(BUILD_DIR)/CMakeCache.txt:
	$(CMAKE) -S . -B $(BUILD_DIR)

build: $(BUILD_DIR)/CMakeCache.txt
	$(CMAKE) --build $(BUILD_DIR) -j $(JOBS)

test: build
	$(CTEST) --test-dir $(BUILD_DIR) --output-on-failure

run: build
	./$(BUILD_DIR)/circus_elephant

clean:
	rm -rf $(BUILD_DIR)

# Code style (.clang-format: Allman, 4 spaces). Override the binary with CLANG_FORMAT=...
CLANG_FORMAT ?= clang-format
format:
	$(CLANG_FORMAT) -i $$(git ls-files '*.cpp' '*.hpp')
