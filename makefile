# Compiler
CC = g++

# Compiler flags
CFLAGS = -g

# Linker flags
LDFLAGS = -lGL -lGLU -lglut -lGLEW -lSOIL

# Source files
SRCS = main.cpp Terrain.cpp Camera.cpp InputHandler.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
EXEC = main

all: $(EXEC) clean_obj

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(EXEC)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

run: $(EXEC) clean_obj
	./$(EXEC)

clean_obj:
	rm -f $(OBJS)

clean:
	rm -f $(OBJS) $(EXEC)
