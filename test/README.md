# VZemu Unit Tests

This directory contains unit tests for the VZemu VCV Rack plugin modules.

## Requirements

- Google Test (gtest) library
- VCV Rack SDK
- **VCV Rack Runtime Library** (libRack.dll or libRack.a)

## Important Note

VCV Rack modules require the Rack runtime library to function. The Rack SDK by itself only contains headers and import libraries. You have two options:

### Option 1: Use VCV Rack Installation (Recommended)

If you have VCV Rack installed, copy `libRack.dll` from your Rack installation to the test directory or add it to your PATH:

```bash
# Find your Rack installation
export RACK_INSTALL="/c/Program Files/VCV/Rack2"  # Adjust path as needed

# Copy the DLL
cp "$RACK_INSTALL/libRack.dll" test/

# Or add to PATH
export PATH="$RACK_INSTALL:$PATH"
```

### Option 2: Build Rack from Source

Build VCV Rack from source following the instructions at:
https://vcvrack.com/manual/Building

This will create the necessary libRack.dll in the Rack build directory.

## Installation

### Install Google Test (Ubuntu/Debian)
```bash
sudo apt-get install libgtest-dev
cd /usr/src/gtest
sudo cmake .
sudo make
sudo cp lib/*.a /usr/lib
```

### Install Google Test (Windows with vcpkg)
```powershell
vcpkg install gtest
```

### Install Google Test (macOS with Homebrew)
```bash
brew install googletest
```

## Building Tests

From the test directory:
```bash
make
```

## Running Tests

After building, run the tests with:
```bash
make test
```

Or run the executable directly:
```bash
./test_envelope       # Linux/macOS
test_envelope.exe     # Windows
```

## Test Coverage

### Envelope Module Tests (`test_Envelope.cpp`)

Tests for the Envelope module include:

- **InitialState**: Verifies module initializes with correct default values
- **ParameterConfiguration**: Checks all parameters are properly configured with correct ranges
- **IOConfiguration**: Validates inputs, outputs, and lights are configured
- **TriggerDetection**: Tests envelope triggering on gate input
- **NoRetriggerWhileHigh**: Ensures envelope doesn't retrigger while gate is held high
- **TriggerReload**: Verifies trigger reload mechanism after gate release
- **NumberOfSteps**: Tests the number of steps parameter and corresponding lights
- **SustainStepSelection**: Validates sustain step button functionality
- **SustainButtonDebounce**: Tests button debouncing behavior
- **RateParameterAffectsStepLength**: Verifies rate parameter controls step timing
- **OutputGeneration**: Tests envelope output voltage generation
- **EnvelopeCompletion**: Ensures envelope completes and resets properly
- **SustainHoldsEnvelope**: Tests that sustain holds the envelope at the specified step
- **OutputZeroWhenInactive**: Verifies output is zero when envelope is not triggered
- **LightConfiguration**: Validates all lights are accessible and functional

## Adding New Tests

To add tests for other modules:

1. Create a new test file: `test_ModuleName.cpp`
2. Add the test source to `TEST_SOURCES` in the Makefile
3. Follow the existing test structure using Google Test framework
4. Update this README with test coverage information

## Test Structure

Each test file should:
- Include necessary headers (`gtest/gtest.h`, module headers)
- Create a test fixture class inheriting from `::testing::Test`
- Implement `SetUp()` and `TearDown()` methods for initialization/cleanup
- Write individual test cases using `TEST_F` macro
- Provide helper methods as needed

## Notes

- Tests assume a sample rate of 44100 Hz by default
- The VCV Rack module interface is used, so tests require Rack SDK
- Tests are designed to be run in isolation and should not depend on each other
