# Linux:  sudo apt install freeglut3-dev   then   make && ./haunted_house
CXX ?= g++
CXXFLAGS ?= -O2 -Wall

SRCS = haunted_house.cpp primitives.cpp train_railway.cpp house_exterior.cpp house_interior.cpp grounds.cpp dynamic_objects.cpp lighting_sky.cpp
OBJS = $(SRCS:.cpp=.o)

ifeq ($(OS),Windows_NT)
  TARGET = haunted_house.exe
  LIBS = -lfreeglut -lglu32 -lopengl32 -static-libgcc -static-libstdc++
else
  UNAME := $(shell uname 2>/dev/null)
  ifeq ($(UNAME),Darwin)
    TARGET = haunted_house
    LIBS = -framework GLUT -framework OpenGL -Wno-deprecated-declarations
  else
    TARGET = haunted_house
    LIBS = -lglut -lGLU -lGL -lm
  endif
endif

all: $(TARGET)

haunted_house: $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
ifeq ($(OS),Windows_NT)
	-cmd /c del /q /f $(TARGET) *.o 2>nul || exit 0
else
	rm -f $(TARGET) $(OBJS)
endif
