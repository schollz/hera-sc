# Makefile for Hera SuperCollider Plugin
# Based on https://github.com/jpcima/Hera

SC_PATH ?= ./supercollider

# Detect if supercollider folder exists
ifeq ($(wildcard $(SC_PATH)/include/plugin_interface/SC_PlugIn.h),)
  $(info SuperCollider source not found at $(SC_PATH))
  $(info Cloning SuperCollider repository...)
  NEED_SC_CLONE := 1
else
  $(info Found SuperCollider source at $(SC_PATH))
  NEED_SC_CLONE := 0
endif

.PHONY: all build clean sc-clone

all: build

sc-clone:
ifeq ($(NEED_SC_CLONE),1)
	@echo "Cloning SuperCollider repository..."
	git clone https://github.com/supercollider/supercollider.git $(SC_PATH)
	cd $(SC_PATH) && git checkout Version-3.14.0
	cd $(SC_PATH) && git submodule update --init --recursive
	@echo "SuperCollider source code ready"
endif

build: sc-clone
	@echo "Building Hera SuperCollider plugin..."
	mkdir -p plugins/Hera/build
	cd plugins/Hera/build && cmake -DSC_PATH=../../../$(SC_PATH) -DCMAKE_BUILD_TYPE=Release ..
	cd plugins/Hera/build && make

clean:
	rm -rf plugins/Hera/build

distclean: clean
	rm -rf $(SC_PATH)
