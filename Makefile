# Standard Makefile for small projects
#
# Depends: clang-formater, doxygen, ctags 
#
BIN=hotword
BINOBJ = hotword.o cmdargs.o alsadev.o
# spdlog can include (bundled) fmt or have it external. In case of external (archlinux), 
# SPDLOG_FMT_EXTERNAL must be defined and the fmt library linked additionally 
CFLAGS=-pthread -DSPDLOG_FMT_EXTERNAL
CXXFLAGS=-pthread 
LDFLAGS=
LDLIBS=-lstdc++ -lspdlog -lfmt -lpthread -lasound


CC  = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++

SRCDIR=src
OBJDIR=obj
DOCDIR=doc

all: $(BIN)
# prefix object names with objectdir
_BINOBJ = $(patsubst %,$(OBJDIR)/%,$(BINOBJ))

# link all binobj to bin
$(BIN): $(_BINOBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# default rule for cpp file
# for a given objet, compile the sourcefile. Create objdir at first
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CC) -c $(INC) -o $@ $< $(CFLAGS) 

$(OBJDIR):
	mkdir -p $@
	
$(DOCDIR):
	mkdir -p $@

documentation: $(DOCDIR)
	doxygen

beautify:
	clang-format -i -sytle=google ./src/*

install: $(BIN)
ifneq (0, $(shell id -u))
	@echo "Installation requires to be root !"
	@exit 1
endif
	mkdir -p $(DESTDIR)/usr/bin
	install -Dm755 "$(BIN)" "$(DESTDIR)/usr/bin"

.PHONY : tags clean beautify

tag:
	ctags --recurse

clean:
	rm -rf obj
	rm -rf ${BIN}
