# Compiler
CC = g++

# Compiler flags
CFLAGS = -O3 -march=native -g -I/home/antonio/.miniconda3/envs/tensorflow/include/python3.7m

# Linker flags
LDFLAGS = -L/home/antonio/.miniconda3/envs/tensorflow/lib -lGL -lGLU -lglut -lGLEW -lSOIL -lassimp -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio -lpython3.7m

# Source files
SRCS = main.cpp Terrain.cpp Camera.cpp InputHandler.cpp GlutFramework.cpp Renderer.cpp

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
	LD_LIBRARY_PATH=/home/antonio/.miniconda3/envs/tensorflow/lib ./$(EXEC)

clean_obj:
	rm -f $(OBJS)

clean:
	rm -f $(OBJS) $(EXEC)
