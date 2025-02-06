# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -lpthread

# Source files
SRC = main.cpp mapper.cpp reducer.cpp
OBJ = $(SRC:.cpp=.o)
EXE = tema1

# Build executable
$(EXE): $(OBJ)
	$(CXX) $(OBJ) -o $(EXE) $(CXXFLAGS)

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJ) $(EXE)
