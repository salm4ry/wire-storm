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

# prog.c -> prog
src = $(wildcard *.c)
obj := $(src:.c=)

all: $(obj)
% : %.c
	$(foreach comm,$(CC),\
		$(if $(shell command -v $(comm) 2>/dev/null),,\
			$(error $(comm) not found, consider installing)))
	$(CC) $< -o $@ $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(obj)
