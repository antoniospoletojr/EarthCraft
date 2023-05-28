# Compiler
CC = g++

# Compiler flags
CFLAGS = -O3 -march=native -g -I/home/antonio/.miniconda3/envs/tensorflow/include/python3.7m -MMD -MP

# Linker flags
LDFLAGS = -L/home/antonio/.miniconda3/envs/tensorflow/lib -lGL -lGLU -lglut -lGLEW -lSOIL -lassimp -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio -lpython3.7m

# Source files
SRCS = main.cpp Terrain.cpp Camera.cpp InputHandler.cpp GlutFramework.cpp Renderer.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Dependency files
DEPS = $(SRCS:.cpp=.d)

# Executable name
EXEC = main

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(EXEC)

-include $(DEPS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

run: $(EXEC)
	LD_LIBRARY_PATH=/home/antonio/.miniconda3/envs/tensorflow/lib ./$(EXEC)

clean:
	rm -f $(OBJS) $(EXEC)

.PHONY: clean
