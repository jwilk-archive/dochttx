EXEFILE = dochttx
HFILES = $(wildcard *.h)
CFILES = $(wildcard *.c)
OFILES = $(CFILES:%.c=%.o)
MAKEFILES = Makefile $(wildcard Makefile.*)
DISTFILES = $(CFILES) $(HFILES) $(MAKEFILES)
FAKEROOT = $(shell command -v fakeroot 2>/dev/null)

VERSION = 0.2
CC = gcc
CFLAGS := -std=gnu99
CFLAGS += -Os -s
CFLAGS += -W -Wall
CFLAGS += -D_GNU_SOURCE -DVERSION='"$(VERSION)"'
LDFLAGS := -lzvbi -lncursesw
INDENTFLAGS = -nut -cli0 -bli0 -npcs -npsl

.PHONY: all
all: $(EXEFILE)

.PHONY: test
test: $(EXEFILE)
	LD_LIBRARY_PATH=./libzvbi ./$(EXEFILE)

.PHONY: clean
clean:
	rm -f *.o $(EXEFILE) core core.* *~

.PHONY: indent
indent:
	indent $(INDENTFLAGS) $(CFILES) $(HFILES)

$(OFILES): %.o: %.c
	$(CC) $(CFLAGS) -c ${<} -o ${@}

$(EXEFILE): $(OFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) ${^} -o ${@}

Makefile.dep: $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -MM $(CFILES) > ${@}

.PHONY: dist
dist:
	$(FAKEROOT) tar -cjf $(EXEFILE)-$(VERSION).tar.bz2 $(DISTFILES)

-include Makefile.dep

# vim:ts=4
