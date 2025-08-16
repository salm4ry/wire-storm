include common/Makefile

# prog.c -> prog
src := $(wildcard *.c)
obj := $(src:.c=)

top_dir := $(PWD)
#include_dir := $(top_dir)/include
#include_files := $(wildcard $(include_dir)/*.c)

lib_dir := $(top_dir)/lib
lib := $(lib_dir)/libwirestorm.so

lib_src := $(wildcard $(lib_dir)/*.c)
lib_src += $(wildcard $(lib_dir)/*.h)

doc_dir := $(top_dir)/doc


man_page := man/ws_server.roff
doxyfile = doc/Doxyfile

all: $(obj)

# from gcc(1):
# -L: add directory to list of directories to be searched for -l (libraries)
# -I: add directory to list of directories to be searched for header files
% : %.c $(lib)
	$(foreach comm,$(CC),\
		$(if $(shell command -v $(comm) 2>/dev/null),,\
			$(error $(comm) not found, consider installing)))
	$(CC) -Llib -Ilib $< -o $@ $(CFLAGS) -lwirestorm

# .so depends on source files in lib directory
$(lib): $(lib_src)
	cd $(lib_dir) && make

.PHONY: help
help:
	@echo 'Compilation:'
	@echo '  all  - build the server program'
	@echo 'Environment Variables:'
	@echo '  SCAN_BUILD - compile with scan-build for static analysis'
	@echo '  LLVM       - use clang instead of gcc for compilation'
	@echo '  DEBUG      - compile in debug mode (-DDEBUG)'
	@echo '  ARGS       - specify command-line arguments to make run'
	@echo 'Documentation:'
	@echo '  docs       - generate docs (HTML and LaTeX) with doxygen'
	@echo '  man        - view manpage'
	@echo 'Cleaning:'
	@echo '  clean      - remove all binaries (including those in lib/)'
	@echo '  clean-docs - clean generated documentation'
	@echo 'Other:'
	@echo '  run        - run the server program'
	@echo '  help       - print this help and exit'

.PHONY: run
run:
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(lib_dir) ./$(obj) $(ARGS)

.PHONY: clean
clean:
	rm -f $(obj)
	cd $(lib_dir) && make clean


.PHONY: docs
docs:
	doxygen $(doxyfile)


.PHONY: clean-docs
clean-docs:
	rm -rf $(doc_dir)/html $(doc_dir)/latex

.PHONY: man
man:
	man -l $(man_page)
