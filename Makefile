all:
	g++ src/*.cpp -o ufpi-surfers -lGLEW -lGL -lGLU -lglut -lassimp

run:
	./ufpi-surfers

clean:
	rm -f ufpi-surfers