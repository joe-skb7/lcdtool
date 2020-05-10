APP := lcdtool
OBJS :=			\
	src/lcdtool.o	\
	src/file.o	\
	src/tools.o
CPPFLAGS := -Iinclude -MD
CFLAGS := -Wall -O2
LDFLAGS :=
LDLIBS :=
CC := gcc
LD := gcc

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
  Q = @
else
  Q =
endif

BUILD ?= debug
ifeq ($(BUILD),release)
  CFLAGS += -DNDEBUG
  LDFLAGS += -s
else ifeq ($(BUILD),debug)
  CFLAGS += -ggdb3
else
  $(error Incorrect BUILD variable)
endif

all: $(APP)

$(APP): $(OBJS)
	@printf "  LD      $(APP)\n"
	$(Q)$(LD) $(LDFLAGS) $^ -o $@ $(LDLIBS)

%.o: %.c
	@printf "  CC      $(*).c\n"
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).c

clean:
	@printf "  CLEAN\n"
	$(Q)-rm -f src/*.o
	$(Q)-rm -f $(APP)

.PHONY: all clean

-include $(OBJS:.o=.d)
