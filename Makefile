all: detect draw

detect: detect.cpp
	g++ $+ -o $@ `pkg-config --libs opencv`
draw: draw.cpp
	g++ $+ -o $@ `pkg-config --libs imlib2 expat`
