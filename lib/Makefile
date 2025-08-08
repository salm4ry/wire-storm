include ../common/Makefile

# optimise library objects with -O2 since they contain frequently-used functions
CFLAGS := $(CFLAGS) -O2

src := $(wildcard *.c)
headers := $(wildcard *.h)
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
%.o: %.c $(headers)
	$(foreach comm,$(CC),\
		$(if $(shell command -v $(comm) 2>/dev/null),,\
			$(error $(comm) not found, consider installing)))
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

.PHONY: clean
clean:
	rm -f $(obj) $(target)
