APP := lcdtool
SOURCES := lcdtool.c file.c tools.c
CFLAGS := -Wall -O2

all: $(SOURCES)
	@printf "  CC      $(APP)\n"
	@$(CC) $(CFLAGS) $(SOURCES) -o $(APP)

clean:
	-rm -f $(APP)

.PHONY: all clean
