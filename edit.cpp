/*****************************************************************
 * edit.cpp
 *
 * Copyright 2012-2014, Hendrik Radke
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the license contained in the
 * COPYING file.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Edit XML files with comic bubbles.
 * Features:
 * - [ ] Add / delete comic bubbles in elliptical and rectangular form
 * - [ ] Edit coordinates / size of comic bubbles
 * - [ ] Edit color / text of comic bubbles
 */

#include "comicfile.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>

// WINDOW CLASS TO HANDLE DRAWING IMAGE
class MyWindow : public Fl_Double_Window {
    unsigned char* pixbuf;              // image buffer
    
    // FLTK DRAW METHOD
    void draw() {
        fl_draw_image((const uchar*)pixbuf, 0, 0, w(), h(), 4, 0);
    }
public:
    MyWindow(int w, int h, const char *name=0) : Fl_Double_Window(w,h,name) {
	pixbuf = (uchar*)malloc(w*h*4);
        end();
    }
    ~MyWindow() {
	free(pixbuf);
    }
    void setImage(DATA32* data) {
	std::cout<< "Setting img of size " <<w()<<"x"<<h()<<"\n";
	for (int i=0; i<w()*h(); i+=1) {
	    pixbuf[4*i+0] = (data[i]>>16) & 0xFF; // b
	    pixbuf[4*i+1] = (data[i]>> 8) & 0xFF; // g
	    pixbuf[4*i+2] = (data[i]>> 0) & 0xFF; // r
	    pixbuf[4*i+3] = (data[i]>>24) & 0xFF; // a
	}
	redraw();
    }
};

int main(int argc, char **argv) {
    if(argc == 2) {
	parse_XML(argv[1]);
    } else {
	std::cerr<< "usage: "<< argv[0] <<" <XML file>\n";
	exit(-1);
    }

    // Create GUI
    MyWindow *window = new MyWindow(imlib_image_get_width(), imlib_image_get_height());
    window->end();
    window->setImage(imlib_image_get_data());
    window->show(argc, argv);
    window->redraw();
    return Fl::run();
}

void Bubble::draw() {
    if (shape == "ellipse") {
	int centerx = arg_i(args, "centerx"),  centery = arg_i(args, "centery");
	int radiusx = arg_i(args, "radiusx"),  radiusy = arg_i(args, "radiusy");
	if (radiusx > centerx || radiusy > centery)
	    std::cerr<< "Warning: Ellipse Radii ("<< radiusx <<", "<< radiusy <<")"
		     << " > Midpoint ("<< centerx <<", "<< centery <<")\n";
	if (radiusx < 8) std::cerr<< "Warning: Ellipse x-Radius "<<radiusx<<" too small.\n";
	if (radiusy < 8) std::cerr<< "Warning: Ellipse y-Radius "<<radiusy<<" too small.\n";
	colors.back().use();
	imlib_image_fill_ellipse(centerx, centery, radiusx-1, radiusy-1);
	imlib_text_draw(centerx, centery+10, "yo");
    } else if (shape == "rectangle") {
	int x0    = arg_i(args, "x0"),    y0 = arg_i(args, "y0");
	int width = arg_i(args, "width"), height = arg_i(args, "height");
	if (width  < 8) std::cerr<< "Warning: Rectangle Width " <<width <<" too small.\n";
	if (height < 8) std::cerr<< "Warning: Rectangle Height "<<height<<" too small.\n";
	colors.back().use();
	imlib_image_fill_rectangle(x0, y0, width, height);
	// Text schreiben
	imlib_text_draw(x0, y0+10, "yo");
    } else {
	std::cerr<< "Error: unknown bubble shape "<< shape <<"\n";
    }
}
