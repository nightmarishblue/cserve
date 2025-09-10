.PHONY: debug

COMPILER_OPTS := -Wall -Wpedantic

TARGET := cserve

SRC_DIR := src/
INC_DIR := inc/
OUT_DIR := bin/
TST_DIR := tests/

# build the program
build: $(OUT_DIR)$(TARGET)

# make the out dir
make_out: $(OUT_DIR)

test: build
	@EXE="./$(OUT_DIR)$(TARGET)" "./$(TST_DIR)/all.sh"

debug: COMPILER_OPTS += -g
debug: clean build

clean:
	rm "$(OUT_DIR)$(TARGET)" || true

$(OUT_DIR)$(TARGET): $(SRC_DIR)* | make_out
	gcc "$(SRC_DIR)"* -I "$(INC_DIR)" -o "$(OUT_DIR)$(TARGET)" $(COMPILER_OPTS)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)