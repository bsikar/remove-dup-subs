CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
LDFLAGS = -lstdc++fs  # Link with filesystem library if needed

TARGET = remove_dup_subs
SRCDIR = src
SRCS = $(SRCDIR)/main.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
        $(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
        $(CXX) $(CXXFLAGS) -c $< -o $@

clean:
        rm -f $(SRCDIR)/*.o $(TARGET)

.PHONY: all clean

