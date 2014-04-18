CXX     = g++
LD      = g++
CFLAGS  = -c -pedantic -Wall -ggdb
LDFLAGS =
SOURCES = lexan.cpp parser.cpp strom.cpp tabsym.cpp vstup.cpp zaspoc.cpp main.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXEC    = pjp
VG      = valgrind
VGFLAGS = -v --leak-check=full --show-reachable=yes --track-origins=yes

all: $(SOURCES) $(EXEC)
	
$(EXEC): $(OBJECTS) 
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CXX) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXEC)

run:
	./$(EXEC)

memcheck:
	$(VG) $(VGFLAGS) ./$(EXEC)
