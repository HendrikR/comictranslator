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

Bubble* current = 0;
Fl_Window* mainWindow;

class MyBox : public Fl_Box {
public:
    enum EditMode { DM_POINT, DM_DRAW_RECT, DM_DRAW_CIRC, DM_DELETE };
    uchar* pixbuf;              // image buffer
    Comicfile* comic;
    Fl_Input* status_bar;
    EditMode editmode;
    int oldx, oldy;

    // FLTK DRAW METHOD
    void draw() {
        fl_draw_image(pixbuf, x(), y(), w(), h(), 4, 0);
	for (unsigned i=0; i<comic->bubbles.size(); i++) {
	    Bubble* bubble = comic->bubbles[i];
	    bubble->draw();
	}
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
    void setStatusBar(Fl_Input* _status_bar) {
	status_bar = _status_bar;
    }
    Bubble* bubbleAt(int x, int y) {
	for (std::vector<Bubble*>::const_iterator b = comic->bubbles.begin();
	     b != comic->bubbles.end();  b++) {
	    Bubble* bubble = *b;
	    if (bubble->contains(x,y)) {
		return bubble;
	    }
	}
	return NULL;
    }
    virtual int handle(int event) {
	int cx = Fl::event_x() - x();
	int cy = Fl::event_y() - y();

	if (event == FL_MOVE) {
	    if (editmode == DM_POINT) {
                Bubble* bubble = bubbleAt(cx, cy);
		if (bubble != NULL) {
		    status_bar->value(bubble->text.c_str());
		    current = bubble;
		    bubble->draw();
		    free(pixbuf);
		    load_image();
		    draw();
		}
	    }
	} else if (event == FL_PUSH) {
	    if (editmode == DM_POINT) {
	    } else if (editmode == DM_DRAW_RECT) {
		oldx = cx;
		oldy = cy;
	    } else if (editmode == DM_DRAW_CIRC) {
		oldx = cx;
		oldy = cy;
	    }
	} else if (event == FL_RELEASE) {
	    if (editmode == DM_POINT) {
	    } else if (editmode == DM_DRAW_RECT) {
		editmode = DM_POINT;
		mainWindow->cursor(FL_CURSOR_DEFAULT);
		if (cx < oldx) std::swap(oldx, cx);
		if (cy < oldy) std::swap(oldy, cy);
		Bubble* bubble = new BubbleRectangle(oldx, oldy, cx-oldx, cy-oldy, comic->getFont("default"), comic->getColor("default"));
		comic->add(bubble);
		bubble->draw();
	    } else if (editmode == DM_DRAW_CIRC) {
		editmode = DM_POINT;
		mainWindow->cursor(FL_CURSOR_DEFAULT);
		if (cx < oldx) std::swap(oldx, cx);
		if (cy < oldy) std::swap(oldy, cy);
		int radx = (cx-oldx)/2,  rady = (cy-oldy)/2;
		Bubble* bubble = new BubbleEllipse(oldx+radx, oldy+rady, radx, rady, comic->getFont("default"), comic->getColor("default"));
		comic->add(bubble);
		bubble->draw();
		redraw();
	    } else if (editmode == DM_DELETE) {
		Bubble* bubble = bubbleAt(cx, cy);
                if (bubble != NULL) {
                    // TODO: remove bubble from comicfile
                }
            }
	} else if (event == FL_DRAG) {
	    fl_color(255, 0, 255);
	    fl_line_style(FL_DASH, 1, const_cast<char*>("\x04\x04"));
	    if (editmode == DM_DRAW_RECT) {
		fl_rect(oldx, oldy+30, abs(cx-oldx), abs(cy-oldy));
		redraw();
	    } else if (editmode == DM_DRAW_CIRC) {
		//fl_pie(centerx-radiusx, centery-radiusy+30, 2*radiusx, 2*radiusy, 0, 360);
		fl_rect(oldx, oldy+30, abs(cx-oldx), abs(cy-oldy));
		redraw();
	    }
	} else if (event == FL_SHORTCUT || FL_HIDE) {
	    // makes ESC terminate the program, but everything else stops working :-(
	    return 1;
	}
	return 1;
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
MyBox* box;

void cb_drawRect(Fl_Widget* widget, void* data) {
    mainWindow->cursor(FL_CURSOR_CROSS);
    box->editmode = MyBox::DM_DRAW_RECT;
}

void cb_drawCirc(Fl_Widget* widget, void* data) {
    mainWindow->cursor(FL_CURSOR_CROSS);
    box->editmode = MyBox::DM_DRAW_CIRC;
}

void cb_delShape(Fl_Widget* widget, void* data) {
    mainWindow->cursor(FL_CURSOR_HAND);
    box->editmode = MyBox::DM_DELETE;
}

void cb_setText(Fl_Widget* widget, void* data) {
    // TODO: get current bubble as data
}

void cb_save(Fl_Widget* widget, void* data) {

}

int main(int argc, char **argv) {
    Comicfile* comic;
    comic->addFontpath("./fonts");
    if(argc == 2) {
	comic = parse_XML(argv[1]);
    } else {
	std::cerr<< "usage: "<< argv[0] <<" <XML file>\n";
	exit(-1);
    }
    comic->draw();

    // Create GUI
    mainWindow = new Fl_Window(imlib_image_get_width(), imlib_image_get_height()+30);
    box = new MyBox(0,30);
    //Fl_Box box(0,30,imlib_image_get_width(), imlib_image_get_height());
    Fl_Button bRect ( 0,0, 40,30, "rect");
    Fl_Button bCirc (40,0, 40,30, "circ");
    Fl_Button bDel  (80,0, 40,30, "del");
    Fl_Input* status_bar = new Fl_Input(120,0,box->w()-200,30, "");
    // TODO: maybe replace this with an "edit" window, where you can set text, size, font, color?
    Fl_Button bSet  (box->w()-80,0, 40,30, "set");
    Fl_Button bSave (box->w()-40,0, 40,30, "save");
    box->setComic(comic);
    box->setStatusBar(status_bar);

    // set callbacks
    bRect.callback(cb_drawRect);
    bCirc.callback(cb_drawCirc);
    bDel.callback(cb_delShape);
    bSet.callback(cb_setText);
    bSave.callback(cb_save);

    // deactivate un-implemented GUI elements (TODOs)
    status_bar->deactivate();
    bDel.deactivate();
    bSet.deactivate();
    bSave.deactivate();

    mainWindow->end();
    //mainWindow->setImage(imlib_image_get_data());
    mainWindow->show();
    mainWindow->redraw();
    return Fl::run();
}

void Comicfile::draw() const {
    // Load the original image
    Imlib_Load_Error err;
    Imlib_Image image = imlib_load_image_with_error_return(imgfile.c_str(), &err);
    if (err != 0) {
	std::cerr<< "Error loading image '"<< imgfile <<"', error code "<< err <<"\n";
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

void BubbleEllipse::draw() const {
    if (this == current) {
	imlib_context_set_color(128,0,0,128);
    } else {
	bgcolor->use();
    }
    imlib_image_fill_ellipse(centerx, centery, radiusx-1, radiusy-1);
    font->use();
    imlib_text_draw(centerx-13, centery-6, "TEXT");
}

void BubbleRectangle::draw() const {
    if (this == current) {
	imlib_context_set_color(128,0,0,128);
    } else {
	bgcolor->use();
    }
    imlib_image_fill_rectangle(x0, y0, width, height);
    font->use();
    imlib_text_draw(x0+0.05*width, y0+0.05*height, "TEXT");
}

