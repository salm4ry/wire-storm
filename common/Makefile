# Environment variables
# use clang instead of gcc
ifeq ($(LLVM), 1)
	CC := clang
else
	CC := gcc
endif

# use scan-build for static analysis
ifeq ($(SCAN_BUILD), 1)
	CC := scan-build $(CC)
endif

# enable debug logging
ifeq ($(DEBUG), 1)
	CFLAGS := $(CFLAGS) -DDEBUG
endif

# set default CFLAGS
CFLAGS := -ggdb -Wall -Wpedantic
