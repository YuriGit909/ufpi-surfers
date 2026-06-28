CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I/usr/include/glm -DGLM_ENABLE_EXPERIMENTAL
LIBS = -lGLEW -lGL -lGLU -lglut -lassimp

all:
	$(CXX) $(CXXFLAGS) src/*.cpp -o ufpi-surfers $(LIBS)

run:
	./ufpi-surfers

clean:
	rm -f ufpi-surfers
