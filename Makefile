ifeq ($(LLVM), 1)
	CC := clang
else
	CC := gcc
endif

ifeq ($(SCAN_BUILD), 1)
	CC := scan-build $(CC)
endif

# NOTE add additional flags here
CFLAGS = -ggdb -Wall -Wpedantic

# enable debug logging
ifeq ($(DEBUG), 1)
	CFLAGS := $(CFLAGS) -DDEBUG
endif

# prog.c -> prog
src := $(wildcard *.c)
obj := $(src:.c=)

top_dir := $(PWD)
include_dir := $(top_dir)/include
include := $(wildcard $(include_dir)/*.c)

all: $(obj)
% : %.c $(include)
	$(foreach comm,$(CC),\
		$(if $(shell command -v $(comm) 2>/dev/null),,\
			$(error $(comm) not found, consider installing)))
	$(CC) $< $(include) -o $@ $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(obj)
