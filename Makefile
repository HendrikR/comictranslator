all: detect draw edit

%.o: %.cpp
	g++ -c $+ -o $@
detect: detect.o
	g++ $+ -o $@ `pkg-config --libs opencv`
draw: draw.o comicfile.o
	g++ $+ -o $@ `pkg-config --libs imlib2 expat`
edit: edit.o comicfile.o
	g++ $+ -o $@ `pkg-config --libs imlib2 expat` -lfltk
test: test.cpp
	g++ $+ -o $@ -lfltk -lfltk_gl -lGL
