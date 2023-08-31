EXEC = detect draw edit
CXXFLAGS = -g -Wall -Wwrite-strings -std=c++11 `pkg-config --cflags opencv4 imlib2 expat`

all: $(EXEC)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $+ -o $@

detect: detect.o
	$(CXX) $(CXXFLAGS) $+ -o $@ `pkg-config --cflags opencv4`
draw: draw.o comicfile.o bubble.o bubble_ellipse.o bubble_rectangle.o cfont.o color.o
	$(CXX) $(CXXFLAGS) $+ -o $@ `pkg-config --libs imlib2 expat`
edit: edit.o comicfile.o bubble.o bubble_ellipse.o bubble_rectangle.o cfont.o color.o
	$(CXX) $(CXXFLAGS) $+ -o $@ `pkg-config --libs imlib2 expat` -lfltk
test: test.cpp
	$(CXX) $(CXXFLAGS) $+ -o $@ -lfltk -lfltk_gl -lGL

clean:
	rm *.o $(EXEC)
