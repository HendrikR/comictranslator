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
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/fl_draw.H>

class MyBox : public Fl_Box {
    uchar* pixbuf;              // image buffer
    Comicfile* comic;
    Fl_Input* status;
    
    // FLTK DRAW METHOD
    void draw() {
        fl_draw_image(pixbuf, x(), y(), w(), h(), 4, 0);
    }
    void load_image() {
	pixbuf = (uchar*)calloc(imlib_image_get_width()*imlib_image_get_height(), 4);
	DATA32* data = imlib_image_get_data();
	for (int i=0; i<w()*h(); i+=1) {
	    pixbuf[4*i+0] = (data[i]>>16) & 0xFF; // b
	    pixbuf[4*i+1] = (data[i]>> 8) & 0xFF; // g
	    pixbuf[4*i+2] = (data[i]>> 0) & 0xFF; // r
	    pixbuf[4*i+3] = (data[i]>>24) & 0xFF; // a
	}
	redraw();
    }
public:
    MyBox(int x0, int y0)
	: Fl_Box(x0,y0, imlib_image_get_width(), imlib_image_get_height()) {
    }
    ~MyBox() {
	free(pixbuf);
    }
    void setComic(Comicfile* _comic) {
	comic = _comic;
	load_image();
    }
    void setStatus(Fl_Input* _status) {
	status = _status;
    }
    virtual int handle(int event) {
	if (event == FL_MOVE) {
	    int cx = Fl::event_x() - x();
	    int cy = Fl::event_y() - y();
	    for (std::vector<Bubble*>::const_iterator b = comic->bubbles.begin();  b != comic->bubbles.end();  b++) {
		if ((*b)->contains(cx,cy)) {
		    //std::cout << (*b)->text << std::endl;
		    status->value((*b)->text.c_str());
		}
	    }
	} else if (event == FL_ENTER) return 1;
	else if (event == FL_LEAVE) return 1;
	else return 0;
    }
};
/*
const uchar* bgra_to_rgb() {
    uint pixels = imlib_image_get_width() * imlib_image_get_height();
    unsigned int* in = imlib_image_get_data();

    uchar* out = (uchar*)calloc(pixels,3);
    for (uint i=0; i < pixels; i++) {
	out[3*i+0] = (in[i] >> 16) & 0xFF;
	out[3*i+1] = (in[i] >>  8) & 0xFF;
	out[3*i+2] = (in[i] >>  0) & 0xFF;
    }
    return out;
}
*/
int main(int argc, char **argv) {
    Comicfile* comic;
    if(argc == 2) {
	comic = parse_XML(argv[1]);
    } else {
	std::cerr<< "usage: "<< argv[0] <<" <XML file>\n";
	exit(-1);
    }
    comic->draw();

    // Create GUI
    Fl_Window *window = new Fl_Window(imlib_image_get_width(), imlib_image_get_height()+30);
    MyBox box(0,30);
    //Fl_Box box(0,30,imlib_image_get_width(), imlib_image_get_height());
    Fl_Button bRect ( 0,0, 40,30, "rect");
    Fl_Button bCirc (40,0, 40,30, "circ");
    Fl_Input* status = new Fl_Input(80,0,box.w()-80,30, "");
    status->deactivate();
    box.setComic(comic);
    box.setStatus(status);
    //const uchar* data = bgra_to_rgb();
    //Fl_RGB_Image img(data, imlib_image_get_width(), imlib_image_get_height());
    //box.image(img);
    window->end();
    //window->setImage(imlib_image_get_data());
    window->show(argc, argv);
    window->redraw();
    return Fl::run();
}

void Comicfile::draw() {
    // Load the original image
    Imlib_Load_Error errno;
    Imlib_Image image = imlib_load_image_with_error_return(imgfile.c_str(), &errno);
    if (errno != 0) {
	std::cerr<< "Error loading image '"<< imgfile <<"'\n";
	exit(-1);
    }
    imlib_context_set_image(image);

    // Derive filename and format for output file
    string filename_ext = imgfile.substr(imgfile.rfind('.')+1);
    imlib_image_set_format(filename_ext.c_str());
    string filename_out = imgfile.substr(0, imgfile.rfind('.')+1) + language + "." + filename_ext;
    imlib_context_set_image(image);

    // Draw all the bubbles
    for (std::vector<Bubble*>::const_iterator b = bubbles.begin();  b != bubbles.end();  b++) {
	(*b)->draw();
    }
}

void BubbleEllipse::draw() {
    bgcolor->use();
    imlib_image_fill_ellipse(centerx, centery, radiusx-1, radiusy-1);
    font->use();
    imlib_text_draw(centerx-13, centery-6, "TEXT");
}

void BubbleRectangle::draw() {
    bgcolor->use();
    imlib_image_fill_rectangle(x0, y0, width, height);
    font->use();
    imlib_text_draw(x0+10, y0+10, "TEXT");
}

