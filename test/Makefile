# Test Makefile for VZemu Unit Tests

# Rack SDK directory
RACK_DIR ?= ../..

# Compiler flags
CXX = g++
CXXFLAGS = -std=c++17 -D_USE_MATH_DEFINES -DENVELOPE_TEST_BUILD -I$(RACK_DIR)/include -I$(RACK_DIR)/dep/include -I.. -g
# Note: You need libRack.dll from a Rack installation. See README.md for details.
LDFLAGS = -L$(RACK_DIR) -L. -lRack -lgtest -lgtest_main -static-libgcc -static-libstdc++ -Wl,-Bstatic -lpthread -Wl,-Bdynamic

# Test sources
TEST_SOURCES = test_Envelope.cpp
SRC_SOURCES = ../src/Envelope.cpp
TEST_OBJECTS = $(patsubst %.cpp,../build/test/%.o,$(TEST_SOURCES))
SRC_OBJECTS = $(patsubst ../src/%.cpp,../build/test/%.o,$(SRC_SOURCES))
TEST_EXECUTABLE = ../build/test/test_envelope

# Build target
all: $(TEST_EXECUTABLE)

$(TEST_EXECUTABLE): $(TEST_OBJECTS) $(SRC_OBJECTS)
	@mkdir -p ../build/test
	$(CXX) $(TEST_OBJECTS) $(SRC_OBJECTS) -o $@ $(LDFLAGS)

../build/test/%.o: ../src/%.cpp
	@mkdir -p ../build/test
	$(CXX) $(CXXFLAGS) -c $< -o $@

../build/test/%.o: %.cpp
	@mkdir -p ../build/test
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run tests
test: $(TEST_EXECUTABLE)
	@echo "Note: Ensure libRack.dll is in your PATH or current directory"
	PATH="/mingw64/bin:/c/Program Files/VCV/Rack2Free:$(RACK_DIR):.:$$PATH" ./$(TEST_EXECUTABLE)

# Clean
clean:
	rm -f $(TEST_OBJECTS) $(SRC_OBJECTS) $(TEST_EXECUTABLE)

.PHONY: all test clean
