# SPDX-License-Identifier: MIT
# Copyright (c) 2019-2023 The Pybricks Authors

# ensure micropython submodule is checked out for building mpy-cross
ifeq ("$(wildcard micropython/README.md)","")
$(info GIT cloning micropython submodule)
$(info $(shell git submodule update --init micropython))
ifeq ("$(wildcard micropython/README.md)","")
$(error failed)
endif
endif

ifeq ("$(OS)","Windows_NT")
HOST_OS = Windows
HOST_CROSS_COMPILE = x86_64-w64-mingw32-
else
HOST_OS = $(shell uname -s)
endif

help:
	@echo "Use 'make <BRICK>' to build a brick."

.PHONY: doc

doc:
	@$(MAKE) -C lib/pbio/doc

clean-doc:
	@$(MAKE) -C lib/pbio/doc clean

all: movehub cityhub technichub primehub essentialhub virtualhub nxt debug ev3rt doc

clean-all: clean-movehub clean-cityhub clean-technichub clean-primehub clean-essentialhub clean-virtualhub clean-nxt clean-debug clean-ev3rt clean-doc

ifeq ($(HOST_OS),Linux)

ev3dev-host: mpy-cross
	@$(MAKE) -C bricks/ev3dev CROSS_COMPILE=

clean-ev3dev-host: clean-mpy-cross
	@$(MAKE) -C bricks/ev3dev clean CROSS_COMPILE=

else

ev3dev-host:
	$(error Building ev3dev for host OS only works on Linux)

clean-ev3dev-host: ev3dev-host

endif

ev3dev-armel:
	@if [ ! -d bricks/ev3dev/build-armel/ports ]; then \
		bricks/ev3dev/docker/setup.sh armel; \
	fi
	@docker start pybricks-ev3dev_armel
	@docker exec --tty pybricks-ev3dev_armel make -C ../../micropython/mpy-cross CROSS_COMPILE= -j`nproc`
	@docker exec --tty pybricks-ev3dev_armel make -j`nproc`

clean-ev3dev-armel:
	@if [ -d bricks/ev3dev/build-armel/ports ]; then \
		@docker start pybricks-ev3dev_armel; \
		docker exec --tty pybricks-ev3dev_armel make -C ../../micropython/mpy-cross clean CROSS_COMPILE=; \
		docker exec --tty pybricks-ev3dev_armel make clean; \
	fi

ev3rt: mpy-cross
	@$(MAKE) -C bricks/ev3rt

clean-ev3rt: clean-mpy-cross
	@$(MAKE) -C bricks/ev3rt clean

movehub: mpy-cross
	@$(MAKE) -C bricks/movehub

clean-movehub: clean-mpy-cross
	@$(MAKE) -C bricks/movehub clean

cityhub: mpy-cross
	@$(MAKE) -C bricks/cityhub

clean-cityhub: clean-mpy-cross
	@$(MAKE) -C bricks/cityhub clean

technichub: mpy-cross
	@$(MAKE) -C bricks/technichub

clean-technichub: clean-mpy-cross
	@$(MAKE) -C bricks/technichub clean

nxt: mpy-cross
	@$(MAKE) -C bricks/nxt

clean-nxt: clean-mpy-cross
	@$(MAKE) -C bricks/nxt clean

primehub: mpy-cross
	@$(MAKE) -C bricks/primehub

clean-primehub: clean-mpy-cross
	@$(MAKE) -C bricks/primehub clean

essentialhub: mpy-cross
	@$(MAKE) -C bricks/essentialhub

clean-essentialhub: clean-mpy-cross
	@$(MAKE) -C bricks/essentialhub clean

virtualhub: mpy-cross
	@$(MAKE) -C bricks/virtualhub CROSS_COMPILE=

clean-virtualhub: clean-mpy-cross
	@$(MAKE) -C bricks/virtualhub clean CROSS_COMPILE=
	@$(MAKE) -C bricks/virtualhub clean DEBUG=1

debug: mpy-cross
	@$(MAKE) -C bricks/debug build/firmware.dfu

clean-debug: clean-mpy-cross
	@$(MAKE) -C bricks/debug clean

mpy-cross:
	@$(MAKE) -C micropython/mpy-cross CROSS_COMPILE=$(HOST_CROSS_COMPILE)

clean-mpy-cross:
	@$(MAKE) -C micropython/mpy-cross clean CROSS_COMPILE=$(HOST_CROSS_COMPILE)
