#
# Created By ITAMAR SHARIFY
#
# our compiler
CC=gcc

# compile arguments
CFLAGS+=-c -g -Wall

# linker flags
LDFLAGS+=


# our source files
SOURCES= server.c
# a macro to define the objects from sources
OBJECTS=$(SOURCES:.c=.o)

# executable name	
EXECUTABLE=server

$(EXECUTABLE):
	@echo "Building target" $@ "..."
	@ $(CC) $(SOURCES)  -o $(EXECUTABLE)


run:
	@echo "run in the new terminals: telnet localhost 6000"
	@gnome-terminal
	@gnome-terminal
	@./server 6000
clean:
	@echo "Cleaning..."
	@ rm -rf *s *o $(EXECUTABLE) $(OBJECTS)

.PHONY: all clean
