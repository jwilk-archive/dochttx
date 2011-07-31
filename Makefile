EXEFILE = dochttx
HFILES = $(wildcard *.h)
CFILES = $(wildcard *.c)
OFILES = $(CFILES:%.c=%.o)

VERSION = $(shell sed -n -e '1 s/.*(\([0-9.]*\)).*/\1/p' < doc/changelog)
CC = gcc
CFLAGS := -std=gnu99
CFLAGS += -Os -s
CFLAGS += -W -Wall
CFLAGS += -D_GNU_SOURCE -DVERSION='"$(VERSION)"'
LDFLAGS := -lzvbi -lncursesw

.PHONY: all
all: $(EXEFILE)

.PHONY: clean
clean:
	$(RM) *.o $(EXEFILE) core core.* *~ Makefile.dep

$(OFILES): %.o: %.c
	$(CC) $(CFLAGS) -c ${<} -o ${@}

$(EXEFILE): $(OFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) ${^} -o ${@}

Makefile.dep: $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) -MM $(CFILES) > ${@}

-include Makefile.dep

# vim:ts=4
