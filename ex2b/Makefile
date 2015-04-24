
# our compiler
CC=gcc

# compile arguments
CFLAGS+=-c -g -Wall

# linker flags
LDFLAGS+= -pthread


# our source files
SOURCES= GenericHashTable.c GenericHashTable.h main.c BellmanFord.h BellmanFord.c
# a macro to define the objects from sources
OBJECTS=$(SOURCES:.c=.o)

# executable name	
EXECUTABLE=prog

$(EXECUTABLE): $(OBJECTS)
	@echo "\n Building target" $@ "..."
	@ $(CC) $(LDFLAGS) $(OBJECTS) -o $@
	@echo "***************************************"
	@echo "*             Welcome!                *"
	@echo "* -------------menu:------------------*"	
	@echo "*      type 'make clean' to delete    *"
	@echo "*           all created files         *"
	@echo "*                                     *"
	@echo "*                ENJOY                *"
	@echo "***************************************"
# a rule for generating object files given their c files
.c.o:
	@echo "*   Compiling" $< "...   "
	@ $(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ 

clean:
	@echo "Cleaning..."
	@ rm -rf *s *o $(EXECUTABLE)

.PHONY: all clean
