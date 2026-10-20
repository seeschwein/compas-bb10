CXX ?= arm-blackberry-qnx8eabi-g++
CXXFLAGS ?= -O2 -g -Wall -Wextra -std=c++17 -pthread -fPIC -fexceptions -frtti

BUILD_DIR := build

PROBES := \
	$(BUILD_DIR)/filesystem_probe \
	$(BUILD_DIR)/posix_probe \
	$(BUILD_DIR)/thread_probe \
	$(BUILD_DIR)/loader_probe

.PHONY: all clean

all: $(PROBES) $(BUILD_DIR)/libprobe.so

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/libprobe.so: plugin.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -shared $< \
		-Wl,-soname,libprobe.so -o $@

clean:
	rm -rf $(BUILD_DIR)
