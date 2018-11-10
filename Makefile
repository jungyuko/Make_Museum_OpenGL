all:
	g++ main.cpp Object.cpp Shader.cpp Camera.cpp -o objloader -I/usr/include/GL -lGL -lGLEW -lglut -lSOIL -std=c++11
