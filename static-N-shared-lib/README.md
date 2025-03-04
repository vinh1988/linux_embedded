# Directory structure:

- bin: Directory for the excutable file.
- obj: Directory for the object files.
- inc: Directory for the header files.
- src: Directory for the C and Cpp files.
- lib: Directory for the static and shared lib.

# Run to build
- make all
# Run to clear
- make clean

## Notes:
- -lhello equals -l$(PRJ_NAME) that link to -l$(BIN_DIR)/$(PRJ_NAME)