# compiler
CC=gcc

# compile arguments
CFLAGS+=-c -g -Wall

# linker flags
LDFLAGS+=

# libraries
LIBS+=

# our source files
SOURCES=ex3.c

# a macro to define the objects from sources
OBJECTS=$(SOURCES:.c=.o)

# executable name
EXECUTABLE=ex3

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

# a rule for generating object files given their c files
.c.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	@echo "Ceaning up *.o Files..."
	rm -rf *s *o $(EXECUTABLE)
	@echo "Done!"

.PHONY: all clean
