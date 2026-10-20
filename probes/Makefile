CXX ?= arm-blackberry-qnx8eabi-g++
CXXFLAGS ?= -O2 -g -Wall -Wextra -std=c++17 -pthread -fPIC -fexceptions -frtti

BUILD_DIR := build

PROBES := \
	$(BUILD_DIR)/filesystem_probe \
	$(BUILD_DIR)/posix_probe \
	$(BUILD_DIR)/thread_probe \
	$(BUILD_DIR)/loader_probe

LIBPROBE := $(BUILD_DIR)/libprobe.so

.PHONY: all clean

all: $(PROBES) $(LIBPROBE)

$(BUILD_DIR):
	mkdir -p $@

# Build each probe executable
$(BUILD_DIR)/%: probes/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ -ldl

# Build the shared probe library (placeholder source provided)
$(LIBPROBE): probes/plugin.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -shared $< \
		-Wl,-soname,$(LIBPROBE) -o $@

# Ensure plugin.cpp exists (create a minimal stub if missing)
probes/plugin.cpp:
	@echo "// Minimal plugin for libprobe.so" > $@
	@echo "extern \"C\" void __attribute__((constructor)) init() {}" >> $@
	@echo "extern \"C\" void __attribute__((destructor)) fini() {}" >> $@
	@echo "int main() {}" >> $@

# Build all probes and the library
build: $(PROBES) $(LIBPROBE)

# Clean generated artefacts
clean:
	rm -rf $(BUILD_DIR)
