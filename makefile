# Compiler
CC = g++

# Compiler flags
CFLAGS = -O3 -march=native -g -I/home/antonio/.miniconda3/envs/tensorflow/include/python3.7m -MMD -MP

# Linker flags
LDFLAGS = -L/home/antonio/.miniconda3/envs/tensorflow/lib -lGL -lGLU -lglut -lGLEW -lSOIL -lassimp -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio -lopenal -lpython3.7m -lsndfile

# Source directory
SRC_DIR = src

# Object directory
OBJ_DIR = obj

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Object files
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Dependency files
DEPS = $(OBJS:.o=.d)

# Executable name
EXEC = main

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(EXEC)

-include $(DEPS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

run: $(EXEC)
	LD_LIBRARY_PATH=/home/antonio/.miniconda3/envs/tensorflow/lib ./$(EXEC)

clean:
	rm -f $(OBJS) $(DEPS) $(EXEC)

.PHONY: clean
