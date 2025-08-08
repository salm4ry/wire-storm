ifeq ($(LLVM), 1)
	CC := clang
else
	CC := gcc
endif

ifeq ($(SCAN_BUILD), 1)
	CC := scan-build $(CC)
endif

# optimise library objects with -O2 since they contain frequently-used functions
CFLAGS = -ggdb -Wall -Wpedantic -O2

# enable debug logging
ifeq ($(DEBUG), 1)
	CFLAGS := $(CFLAGS) -DDEBUG
endif

src := $(wildcard *.c)
obj := $(src:.c=.o)

# top directory = parent directory
top_dir := $(abspath $(notdir $(abspath $(PWD)/..)))
target = libwirestorm.so

all: $(target)

.PHONY: help
help:
	@echo 'Compilation:'
	@echo '  all    - build the object and library files'
	@echo 'Cleaning:'
	@echo '  clean  - remove object and library files'

# create shared object
$(target): $(obj)
	$(CC) $(CFLAGS) -shared -o $(target) $(obj)

# create object files
%.o: %.c
	$(foreach comm,$(CC),\
		$(if $(shell command -v $(comm) 2>/dev/null),,\
			$(error $(comm) not found, consider installing)))
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

.PHONY: clean
clean:
	rm -f $(obj) $(target)
