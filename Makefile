EXEC = detect draw edit
CXXFLAGS = -g -Wall -Wwrite-strings -std=c++11


all: $(EXEC)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $+ -o $@

detect: detect.o
	$(CXX) $(CXXFLAGS) $+ -o $@ `pkg-config --libs imlib2` -lopencv_core -lopencv_imgproc -lopencv_imgcodecs
draw: draw.o comicfile.o bubble.o cfont.o color.o
	$(CXX) $(CXXFLAGS) $+ -o $@ `pkg-config --libs imlib2 expat`
edit: edit.o comicfile.o bubble.o cfont.o color.o
	$(CXX) $(CXXFLAGS) $+ -o $@ `pkg-config --libs imlib2 expat` -lfltk
test: test.cpp
	$(CXX) $(CXXFLAGS) $+ -o $@ -lfltk -lfltk_gl -lGL

clean:
	rm *.o $(EXEC)
