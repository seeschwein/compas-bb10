# Makefile for Compas BB10 project
# Orchestrates probe builds, libprobe library, and BAR packaging.

BUILD_DIR ?= build
PROBES = filesystem_probe posix_probe thread_probe
LIBPROBE = libprobe.so

# The cross‑compiler can be overridden; default empty for local builds.
CXX ?= arm-blackberry-qnx8eabi-g++
CXXFLAGS ?= -O2 -g -Wall -Wextra -std=c++17 -pthread -fPIC -fexceptions -frtti

.PHONY: all clean build-probes package-bar

all: build-probes $(LIBPROBE)

$(BUILD_DIR):
	@mkdir -p $@

# Build each probe executable
$(BUILD_DIR)/%: probes/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

# Build the shared probe library (placeholder source provided)
$(BUILD_DIR)/$(LIBPROBE): probes/plugin.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -shared $< -Wl,-soname,$(LIBPROBE) -o $@

# Ensure plugin.cpp exists (create a minimal stub if missing)
probes/plugin.cpp:
	@echo "// Minimal plugin for libprobe.so" > $@
	@echo "extern \"C\" void __attribute__((constructor)) init() {}" >> $@
	@echo "extern \"C\" void __attribute__((destructor)) fini() {}" >> $@
	@echo "int main() {}" >> $@

# Build all probes and the library
build-probes: $(patsubst probes/%.cpp,$(BUILD_DIR)/%,$(wildcard probes/*.cpp)) $(BUILD_DIR)/$(LIBPROBE)

# Clean generated artefacts
clean:
	@rm -rf $(BUILD_DIR)

# Stub target representing BAR packaging.
# In a full toolchain this would invoke blackberry‑deploy or docker‑builder.sh.
package-bar: clean build-probes
	@echo "=== BAR packaging stub ==="
	@echo "Copy built artefacts to ./bar/ for later deployment:"
	@mkdir -p bar/$(BUILD_DIR)
	@cp $(BUILD_DIR)/$(PROBES) $(BUILD_DIR)/$(LIBPROBE) bar/$(BUILD_DIR)/
	@echo "Packaging complete. (Run blackberry‑deploy or docker‑builder.sh to finish.)"

# Convenience: run the full workflow with one command
workflow: package-bar
