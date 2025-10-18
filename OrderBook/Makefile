# Makefile for OrderBook project
# Alternative to CMake for quick builds

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
RELEASE_FLAGS = -O3 -march=native -DNDEBUG
DEBUG_FLAGS = -g -O0

TARGET = orderbook
SOURCES = order_book.cpp main.cpp
HEADERS = order.h order_book.h memory_pool.h
OBJECTS = $(SOURCES:.cpp=.o)

# Default target: build release version
.PHONY: all
all: release

# Release build
.PHONY: release
release: CXXFLAGS += $(RELEASE_FLAGS)
release: $(TARGET)
	@echo "Built release version: $(TARGET)"

# Debug build
.PHONY: debug
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)
	@echo "Built debug version: $(TARGET)"

# Link object files
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the program
.PHONY: run
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts
.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS)
	@echo "Cleaned build artifacts"

# Benchmark mode (with timing)
.PHONY: benchmark
benchmark: release
	@echo "Running benchmark..."
	time ./$(TARGET)

# Help
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  make          - Build release version (default)"
	@echo "  make release  - Build optimized release version"
	@echo "  make debug    - Build debug version with symbols"
	@echo "  make run      - Build and run the program"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make benchmark - Run with timing information"
	@echo "  make help     - Show this help message"


