# Makefile for polytopes-process

# Directories
SRC_DIR = src
PALP_DIR = thirdparty/PALP
BUILD_DIR = build

# Compiler and flags
CC = gcc
CFLAGS = -O3 -Wall -Wextra -std=c99
OPTIMIZE_CFLAGS = -O3 -march=native -mtune=native -ffast-math -funroll-loops -finline-functions -flto
INCLUDES = -I$(PALP_DIR)

# PALP source files needed for compilation
PALP_SOURCES = Coord.c Rat.c Vertex.c Polynf.c LG.c
PALP_OBJECTS = $(PALP_SOURCES:.c=.o)

# Target executable
TARGET = polytopes-process

# Source files
MAIN_SRC = $(SRC_DIR)/main.c

# Object files with paths
PALP_OBJ_PATHS = $(addprefix $(PALP_DIR)/, $(PALP_OBJECTS))

.PHONY: all clean palp-objects

all: $(TARGET)

# Build the main target
$(TARGET): $(MAIN_SRC) palp-objects
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(MAIN_SRC) $(PALP_OBJ_PATHS)

# Build PALP object files
palp-objects:
	@echo "Building PALP dependencies..."
	@$(MAKE) -C $(PALP_DIR) $(PALP_OBJECTS)

# Clean build artifacts
clean:
	rm -f $(TARGET) $(PARQUET_TO_TEXT)
	@$(MAKE) -C $(PALP_DIR) clean

# Clean everything including PALP objects
cleanall: clean
	@$(MAKE) -C $(PALP_DIR) cleanall

# Install (optional - copies to /usr/local/bin)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

# Uninstall
uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# Parquet to text converter
PARQUET_TO_TEXT = parquet-to-text
PARQUET_TO_TEXT_SRC = $(SRC_DIR)/parquet_to_text.c

# PKG-CONFIG for Arrow and Parquet GLib libraries
ARROW_CFLAGS = $(shell pkg-config --cflags arrow-glib parquet-glib)
ARROW_LIBS = $(shell pkg-config --libs arrow-glib parquet-glib)

$(PARQUET_TO_TEXT): $(PARQUET_TO_TEXT_SRC)
	$(CC) $(CFLAGS) $(ARROW_CFLAGS) -o $(PARQUET_TO_TEXT) $(PARQUET_TO_TEXT_SRC) $(ARROW_LIBS)

# Help
help:
	@echo "Available targets:"
	@echo "  all                        - Build the main executable (default)"
	@echo "  parquet-to-text            - Build the parquet to text converter"
	@echo "  clean                      - Remove executables"
	@echo "  cleanall                   - Remove executable and all PALP objects"
	@echo "  debug                      - Build with debug symbols"
	@echo "  install                    - Install to /usr/local/bin"
	@echo "  uninstall                  - Remove from /usr/local/bin"
	@echo "  help                       - Show this help message"