# Top-level Makefile for the Compas BB10 project.
#
# This is a thin orchestrator.  Each subproject has its own build system:
#
#   probes/  -- CMake project producing the probe runner + probe binaries
#   (root)   -- CMake project producing the Compas Maps application
#
# Everything is built with the XIAZY Clang toolchain inside Docker:
#
#   make build         # build the map application
#   make build-probes  # build the probe suite
#   make package       # package the map application BAR
#   make package-probes# package the probe suite BAR
#   make deploy BAR=...# deploy a BAR to a device in Development Mode
#   make clean         # remove all build artefacts
#
# Override TOOLCHAIN_IMAGE to use a different container image.

TOOLCHAIN_IMAGE ?= ghcr.io/xiazy/blackberry10-toolchain:latest
ROOT_DIR        := $(CURDIR)
BUILD_DIR       ?= build

BAR             ?= compas.bar
DEVICE_IP       ?=
DEVICE_PASSWORD ?=

.PHONY: all build build-probes package package-probes deploy clean

all: build

# ---------------------------------------------------------------------------
# Build
# ---------------------------------------------------------------------------

build:
	docker run --rm -v "$(ROOT_DIR)":/src -w /src $(TOOLCHAIN_IMAGE) bb10-build /src

build-probes:
	docker run --rm -v "$(ROOT_DIR)/probes":/src -w /src $(TOOLCHAIN_IMAGE) bb10-build /src

# ---------------------------------------------------------------------------
# Package
#
# The BAR packager runs on the host directly; the descriptor's asset paths
# are relative to the descriptor's own directory, so the working directory
# matters.
# ---------------------------------------------------------------------------

package: build
	docker run --rm -v "$(ROOT_DIR)":/src -w /src/bar $(TOOLCHAIN_IMAGE) \
		blackberry-nativepackager -devMode -package "$(BAR)" bar-descriptor.xml

package-probes: build-probes
	docker run --rm -v "$(ROOT_DIR)":/src -w /src/probes $(TOOLCHAIN_IMAGE) \
		blackberry-nativepackager -devMode -package "compas-probes.bar" bar-descriptor.xml

# ---------------------------------------------------------------------------
# Deploy
# ---------------------------------------------------------------------------

deploy:
	@test -n "$(DEVICE_IP)" || (echo "DEVICE_IP is required" && exit 1)
	docker run --rm --network host -v "$(ROOT_DIR)":/src -w /src $(TOOLCHAIN_IMAGE) \
		blackberry-deploy -installApp -device $(DEVICE_IP) -password $(DEVICE_PASSWORD) \
		/src/$(BAR)

# ---------------------------------------------------------------------------
# Clean
# ---------------------------------------------------------------------------

clean:
	rm -rf $(BUILD_DIR) probes/build
