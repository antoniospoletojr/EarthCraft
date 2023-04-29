# Compiler
CC = g++

# Compiler flags
CFLAGS = -g

# Linker flags
LDFLAGS = -lGL -lGLU -lglut -lGLEW -lSOIL

# Source files
SRCS = template.cpp terrain.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
EXEC = template

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(EXEC)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC) 

run: $(EXEC)
	./$(EXEC)
