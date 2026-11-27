# Thin wrapper over CMake. Override the cmake binary with `make CMAKE=/path/to/cmake`.
CMAKE     ?= cmake
CTEST     ?= $(patsubst %cmake,%ctest,$(CMAKE))
BUILD_DIR ?= build
JOBS      ?= $(shell nproc 2>/dev/null || echo 4)

.PHONY: all build test run shots clean format bump-dry bump-version

all: build

$(BUILD_DIR)/CMakeCache.txt:
	$(CMAKE) -S . -B $(BUILD_DIR)

build: $(BUILD_DIR)/CMakeCache.txt
	$(CMAKE) --build $(BUILD_DIR) -j $(JOBS)

test: build
	$(CTEST) --test-dir $(BUILD_DIR) --output-on-failure

run: build
	./$(BUILD_DIR)/circus_elephant

# Scripted runs from tests/scripts/<mini-game>/ -> PNG frames in
# $(BUILD_DIR)/shots/<mini-game>/ (CEL-00023); menu screens -> $(BUILD_DIR)/shots/screens/.
SHOTS_DIR ?= $(BUILD_DIR)/shots
shots: build
	@for dir in tests/scripts/*/; do \
		game=$$(basename $$dir); \
		mkdir -p $(SHOTS_DIR)/$$game; \
		for script in $$dir*.txt; do \
			echo "shots: $$script"; \
			./$(BUILD_DIR)/circus_elephant --minigame $$game --script $$script \
				--shots $(SHOTS_DIR)/$$game || exit 1; \
		done; \
	done
	@echo "shots: menu screens"
	@./$(BUILD_DIR)/circus_elephant --screens $(SHOTS_DIR)/screens
	@ls -R $(SHOTS_DIR)

clean:
	rm -rf $(BUILD_DIR)

# Version bump: Makefile.version + CHANGELOG.md + TODO.md, one commit. TYPE=patch|minor|major.
bump-dry:
	python3 scripts/bump.py $(or $(TYPE),patch) --dry-run

bump-version:
	python3 scripts/bump.py $(or $(TYPE),patch)

# Code style (.clang-format: Allman, 4 spaces). Override the binary with CLANG_FORMAT=...
CLANG_FORMAT ?= clang-format
format:
	$(CLANG_FORMAT) -i $$(git ls-files '*.cpp' '*.hpp')
