lib := libuthread.a
objs := queue.o uthread.o context.o preempt.o

all: $(lib)

CC = gcc

CFLAGS := -Wall -Wextra -g

CFLAGS	+= -MMD

ifneq ($(D),1)
CFLAGS	+= -O2
else
CFLAGS	+= -g
endif

deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)

$(lib): $(objs)
	$(Q) ar rcs $(lib) $(objs)

%.o: %.c 
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(Q) rm -f $(lib) $(objs) $(deps)