# compiler
CC=gcc

# compile arguments
CFLAGS+=-c -g -Wall

# linker flags
LDFLAGS+=

# libraries
LIBS+=-lpthread

# our source files
SOURCES=main.c HashTable.c CreateDataBase.c router.c 

# a macro to define the objects from sources
OBJECTS=$(SOURCES:.c=.o)

# executable name
EXECUTABLE=bf

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
