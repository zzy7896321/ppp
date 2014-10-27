SHELL = /bin/sh

CC = gcc
CFLAGS += -g -std=gnu99 -DDEBUG -DENABLE_MEM_PROFILE
ARFLAGS = cvr

OBJS := 

srcdir := infer query parse .
bindir := bin

LIBPPP := libppp.a

.DEFAULT_GOAL := all

include common/makefile.mk	# AND COMMON_HEADERS
include test/makefile.mk

all: $(LIBPPP) $(TESTS) 

include $(srcdir:%=%/makefile.mk)

$(LIBPPP): $(LIBPPP)($(OBJS))
$(LIBPPP)($(OBJS)):

$(OBJS): %.o: debug.h defs.h ppp.h

$(bindir):
	mkdir $(bindir)

clean:
	rm -rf $(bindir) $(LIBPPP) *.o

.PHONY: all clean

