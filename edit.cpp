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
#include <assert.h>

Bubble* current = 0;
Fl_Window* mainWindow;

void cb_drawRect(Fl_Widget* widget, void* data);
void cb_drawCirc(Fl_Widget* widget, void* data);
void cb_delShape(Fl_Widget* widget, void* data);
void cb_setText(Fl_Widget* widget, void* data);
void cb_save(Fl_Widget* widget, void* data);

class MyBox : public Fl_Box {
public:
    enum EditMode { DM_POINT, DM_DRAW_RECT, DM_DRAW_CIRC };
    uchar* pixbuf;              // image buffer
    Comicfile* comic;
    Fl_Input* text_bar;
    EditMode editmode = DM_POINT;
    int oldx, oldy;

    // FLTK DRAW METHOD
    void draw() {
        fl_draw_image(pixbuf, x(), y(), w(), h(), 4, 0);
	for (unsigned i=0; i<comic->bubbles.size(); i++) {
	    Bubble* bubble = comic->bubbles[i];
	    bubble->draw(Bubble::FILL);
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
        // Create GUI
        auto bRect = new Fl_Button( 0,0, 40,30, "rect");
        auto bCirc = new Fl_Button(40,0, 40,30, "circ");
        auto bDel  = new Fl_Button(80,0, 40,30, "del");
        text_bar = new Fl_Input(120,0, w()-200,30, "");
        // TODO: maybe replace this with an "edit" window, where you can set text, size, font, color?
        auto bSet  = new Fl_Button(w()-80,0, 40,30, "set");
        auto bSave = new Fl_Button(w()-40,0, 40,30, "save");

        // set callbacks
        bRect->callback(cb_drawRect, this);
        bCirc->callback(cb_drawCirc, this);
        bDel->callback(cb_delShape, this);
        bSet->callback(cb_setText, this);
        bSave->callback(cb_save, this);
    }
    ~MyBox() {
	free(pixbuf);
    }
    void setComic(Comicfile* _comic) {
	comic = _comic;
	load_image();
    }
    Bubble* bubbleAt(int x, int y) {
        for (Bubble* bubble : comic->bubbles ) {
	    if (bubble->contains(x,y)) {
		return bubble;
	    }
	}
	return nullptr;
    }

    int handle_move(int event, int cx, int cy) {
        if (editmode == DM_POINT) {
        }
        return 1;
    }

    int handle_push(int event, int cx, int cy) {
	Bubble* bubble = bubbleAt(cx, cy);
        switch (editmode) {
        case DM_POINT:
            if (bubble != NULL) {
                text_bar->value(bubble->text.c_str());
                current = bubble;
                bubble->draw(Bubble::OUTLINE);
                free(pixbuf);
                load_image();
                draw();
            }
	    break;
        case DM_DRAW_RECT: // fallthrough
        case DM_DRAW_CIRC:
            oldx = cx;
            oldy = cy;
            break;
        }
        return 1;
    }

    int handle_release(int event, int cx, int cy) {
        Bubble* bubble;
        switch (editmode) {
        case DM_POINT: break;
        case DM_DRAW_RECT: // fallthrough
        case DM_DRAW_CIRC:
            if (cx < oldx) std::swap(oldx, cx);
            if (cy < oldy) std::swap(oldy, cy);
            if ( editmode == DM_DRAW_RECT ) {
                bubble = new BubbleRectangle(oldx, oldy, cx-oldx, cy-oldy, comic->getFont("default"), comic->getColor("default"));
            } else if ( editmode == DM_DRAW_CIRC ) {
                int radx = (cx-oldx)/2,  rady = (cy-oldy)/2;
                bubble = new BubbleEllipse(oldx+radx, oldy+rady, radx, rady, comic->getFont("default"), comic->getColor("default"));
            } else {
                std::cerr<< "Invalid shape creation\n";
                exit(-1);
            }
            comic->add(bubble);
            bubble->draw(Bubble::FILL);
            current = bubble;
            editmode = DM_POINT;
            mainWindow->cursor(FL_CURSOR_DEFAULT);
            break;
        }
        return 1;
    }

    int handle_drag(int event, int cx, int cy) {
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
        return 1;
    }

    virtual int handle(int event) {
	int cx = Fl::event_x() - x();
	int cy = Fl::event_y() - y();

        switch(event) {
        case FL_MOVE:    return handle_move   (event, cx, cy);
        case FL_PUSH:    return handle_push   (event, cx, cy);
        case FL_RELEASE: return handle_release(event, cx, cy);
        case FL_DRAG:    return handle_drag   (event, cx, cy);
        case FL_SHORTCUT: // fallthrough
        case FL_HIDE:
	    // makes ESC terminate the program, but everything else stops working :-(
	    return 1;
        default: return 1;
	}
    }
};

void cb_drawRect(Fl_Widget* widget, void* data) {
    MyBox* my_box = static_cast<MyBox*>(data);
    assert( my_box != nullptr );
    mainWindow->cursor(FL_CURSOR_CROSS);
    my_box->editmode = MyBox::DM_DRAW_RECT;
}

void cb_drawCirc(Fl_Widget* widget, void* data) {
    MyBox* my_box = static_cast<MyBox*>(data);
    assert( my_box != nullptr );
    mainWindow->cursor(FL_CURSOR_CROSS);
    my_box->editmode = MyBox::DM_DRAW_CIRC;
}

void cb_delShape(Fl_Widget* widget, void* data) {
    if ( !current ) return;
    MyBox* my_box = static_cast<MyBox*>(data);
    assert( my_box != nullptr );
    my_box->comic->del( current );
    current = nullptr;
    my_box->comic->draw();
}

void cb_setText(Fl_Widget* widget, void* data) {
    if ( !current ) return;
    MyBox* my_box = static_cast<MyBox*>(data);
    assert( my_box != nullptr );
    current->setText( my_box->text_bar->value() );
}

void cb_save(Fl_Widget* widget, void* data) {
    MyBox* my_box = static_cast<MyBox*>(data);
    assert( my_box != nullptr );
    // todo: write to file instead of cout
    my_box->comic->writeXML(std::cout);
}

int main(int argc, char **argv) {
    Comicfile::addFontpath("./fonts");
    Comicfile* comic;
    if(argc == 2) {
	comic = parse_XML(argv[1]);
    } else {
	std::cerr<< "usage: "<< argv[0] <<" <XML file>\n";
	exit(-1);
    }
    comic->draw();

    // Create GUI
    mainWindow = new Fl_Window(imlib_image_get_width(), imlib_image_get_height()+30);
    MyBox* box = new MyBox(0,30);
    box->setComic(comic);

    mainWindow->end();
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

void BubbleEllipse::draw(Bubble::DrawMode mode) const {
    switch(mode) {
    case OUTLINE:
	imlib_context_set_color(128,0,0,128);
	break;
    case FILL:
	bgcolor->use();
    case ALL:
	bgcolor->use();
    }
    imlib_image_fill_ellipse(centerx, centery, radiusx-1, radiusy-1);
    font->use();
    imlib_text_draw(centerx-13, centery-6, "TEXT");
}

void BubbleRectangle::draw(Bubble::DrawMode mode) const {
    switch(mode) {
    case OUTLINE:
	imlib_context_set_color(128,0,0,128);
	break;
    case FILL:
	bgcolor->use();
    case ALL:
	bgcolor->use();
    }
    imlib_image_fill_rectangle(x0, y0, width, height);
    font->use();
    imlib_text_draw(x0+0.05*width, y0+0.05*height, "TEXT");
}

