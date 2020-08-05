# Standard Makefile for small projects
#
# Depends: clang-formater, doxygen, ctags 
#
BIN=hotword
BINOBJ = hotword.o cmdargs.o audio.o timer.o net.o
# spdlog can include (bundled) fmt or have it external. In case of external (archlinux), 
# SPDLOG_FMT_EXTERNAL must be defined and the fmt library linked additionally 
CFLAGS=-pthread -DSPDLOG_FMT_EXTERNAL 
CXXFLAGS=-pthread 
# Note: -Wl,-rpath=/a/library/no/in/default/path : adds a path to the lib into binary
LDFLAGS=
LDLIBS=-lstdc++ -lspdlog -lfmt -lpthread -lasound -lsndfile

# external dependencies can be temporarily added to the ./external dir and used without install
# CFLAGS+=-I external/porcupine/include
# LDFLAGS=-L external/porcupine/lib/linux/x86_64/ 
# LDFLAGS+=-Wl,-rpath=/home/smokie/myDev/hotword/external/porcupine/lib/linux/x86_64
LDLIBS+=-lpv_porcupine 

LDLIBS+=-lfvad

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

# create compile_commands.json file by running 'bear' as a wrapper for make
# clean build required to catch all dependencies
compile_commands: clean
	bear make

install: $(BIN)
ifneq (0, $(shell id -u))
	@echo "Installation requires to be root !"
	@exit 1
endif
	mkdir -p $(DESTDIR)/usr/bin
	install -Dm755 "$(BIN)" "$(DESTDIR)/usr/bin"
	mkdir -p $(DESTDIR)/usr/share/hotword/res
	install -m 644 res/beep_hi.wav $(DESTDIR)/usr/share/hotword/res/beep_hi.wav
	install -m 644 res/beep_lo.wav $(DESTDIR)/usr/share/hotword/res/beep_lo.wav
	install -m 644 res/beep_error.wav $(DESTDIR)/usr/share/hotword/res/beep_error.wav


.PHONY : tags clean beautify

tag:
	ctags --recurse

clean:
	rm -rf obj
	rm -rf ${BIN}
	rm -rf compile_commands.json
	rm -rf *.plist
	rm -rf .clangd
	rm -rf tags
