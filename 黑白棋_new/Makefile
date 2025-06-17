CXX = g++
CXXFLAGS = -std=c++17 -Wall
LDFLAGS = -lgdi32
TARGET = reversi.exe

SRCS = main.cpp menu.cpp reversi_game.cpp ai.cpp graphics.cpp menu.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -mwindows -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	del /Q *.o *.exe