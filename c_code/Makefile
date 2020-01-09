PROG     = testcamera
SRCS     = main.c

OBJS     = $(SRCS:.c=.o)
CFLAGS = -O2 -Wall -Werror -ansi `pkg-config gstreamer-1.0 --cflags`
LDLIBS   = -lpthread `pkg-config gstreamer-1.0 --libs`
INCLUDE  =
DEPFILE  = deps.mak

.PHONY: clean

$(PROG): $(OBJS)
		$(CC) $^ $(LDLIBS) -o $@

# How to make the object files:
.c.o:
		$(CC) $(CFLAGS) -c $<

deps:
		$(CC) -MM $(CCFLAGS) $(SRCS) > $(DEPFILE)

# Cleaning target (only works with fileutils):
clean:
		$(RM) $(OBJS) $(PROG)

-include $(DEPFILE)
