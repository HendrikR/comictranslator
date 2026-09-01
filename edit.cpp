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
#include <fstream>

Fl_Window* mainWindow;
Bubble* current;

void cb_drawRect(Fl_Widget* widget, void* data);
void cb_drawCirc(Fl_Widget* widget, void* data);
void cb_delShape(Fl_Widget* widget, void* data);
void cb_setText(Fl_Widget* widget, void* data);
void cb_save(Fl_Widget* widget, void* data);

class MyBox : public Fl_Box {
public:
    enum EditMode { DM_HOVER, DM_DRAW, DM_MOVE, DM_RESIZE, DM_PICK };
    enum EditSubMode { DSM_RECT, DSM_CIRC };
    uchar* img_original;              // unchanged image buffer
    uchar* img_display;              // displayed image buffer
    Comicfile* comic;
    string filename_in, filename_out;
    Fl_Input* text_bar;
    EditMode editmode = DM_HOVER;
    EditSubMode submode = DSM_RECT;
    int oldx, oldy;

    // FLTK DRAW METHOD
    void draw() {
        comic->draw();
        load_image();
        fl_draw_image(img_display, x(), y(), w(), h(), 4, 0); // todo: sometimes segfaults
    }

    void load_image() {
	size_t img_size = imlib_image_get_width() * imlib_image_get_height() * 4;
	if (img_original == nullptr) img_original = (uchar*)malloc(img_size);
	if (img_display == nullptr) img_display = (uchar*)malloc(img_size);
	DATA32* data = imlib_image_get_data();
        // TODO: assumes that image and box size are always the same!
	for (unsigned i=0; i<w()*h(); ++i) {
	    img_original[4*i+0] = (data[i]>>16) & 0xFF; // b
	    img_original[4*i+1] = (data[i]>> 8) & 0xFF; // g
	    img_original[4*i+2] = (data[i]>> 0) & 0xFF; // r
	    img_original[4*i+3] = (data[i]>>24) & 0xFF; // a
	}
	memcpy(img_display, img_original, img_size);
    }

    MyBox(int x0, int y0)
	: Fl_Box(x0,y0, imlib_image_get_width(), imlib_image_get_height()),
          img_original(nullptr), img_display(nullptr)
        {
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
	free(img_original);
	free(img_display);
    }

    void setComic(Comicfile* _comic) {
	comic = _comic;
	load_image();
    }

    Bubble* bubbleAt(int x, int y) const {
        for (Bubble* bubble : comic->bubbles ) {
	    if (bubble->contains(x,y)) {
		return bubble;
	    }
	}
	return nullptr;
    }

    int handle_hover(int event, int cx, int cy) {
        Bubble* bubble = bubbleAt(cx, cy);
        if (bubble == nullptr) {
            // todo: this still redraws every time the cursor moves
            if (current != nullptr) current->draw(); // redraw last selected bubble
            return 1;
        }
        switch(editmode) {
        case DM_HOVER:
            if (current != nullptr && current != bubble) current->draw(); // redraw last selected bubble
            text_bar->value(bubble->getText().c_str());
            current = bubble;
            redraw();
            bubble->draw(Bubble::OUTLINE);
            break;
        case DM_DRAW:
            break;
        default:
            throw std::runtime_error("invalid editmode on hover");
        }
        return 1;
    }

    int handle_press(int event, int cx, int cy) {
	Bubble* bubble = bubbleAt(cx, cy);
        switch (editmode) {
        case DM_HOVER:
            if (bubble != NULL) {
                text_bar->value(bubble->getText().c_str());
                current = bubble;
                oldx = cx; oldy = cy;
                editmode = DM_MOVE;
                redraw();
                bubble->draw(Bubble::OUTLINE);
            }
	    break;
        case DM_DRAW: // create rectangle/ellipse
            oldx = cx;
            oldy = cy;
            break;
        default:
            throw std::runtime_error("invalid editmode on click");
        }
        return 1;
    }

    int handle_release(int event, int cx, int cy) {
        switch(editmode) {
        case DM_HOVER:
            if (current != nullptr) current->draw(Bubble::ALL);
            return 1;
        case DM_MOVE:
            assert(current != nullptr);
            current->draw(Bubble::ALL);
            break;
        case DM_DRAW: {
            if (cx < oldx) std::swap(oldx, cx);
            if (cy < oldy) std::swap(oldy, cy);
            assert(submode == DSM_RECT || submode == DSM_CIRC);
            if (submode == DSM_RECT) {
                current = new BubbleRectangle(oldx, oldy, cx-oldx, cy-oldy, comic->getFont("default"), comic->getColor("default"));
            } else if (submode == DSM_CIRC) {
                int radx = (cx-oldx)/2,  rady = (cy-oldy)/2;
                current = new BubbleEllipse(oldx+radx, oldy+rady, radx, rady, comic->getFont("default"), comic->getColor("default"));
            }
            comic->add(current);
            current->setText("noi");
            current->draw(Bubble::ALL);
            break;
        }
        default:
            throw std::runtime_error("Invalid shape creation");
	}
	editmode = DM_HOVER;
	mainWindow->cursor(FL_CURSOR_DEFAULT);
        return 1;
    }

    int handle_drag(int event, int cx, int cy) {
        switch(editmode) {
        case DM_DRAW:
            // todo this is not visible
            if (submode == DSM_RECT) {
                fl_color(255, 0, 255);
                fl_line_style(FL_DASH, 1, const_cast<char*>("\x04\x04"));
                fl_rect(oldx, oldy+30, abs(cx-oldx), abs(cy-oldy));
            } else if (submode == DSM_CIRC) {
                int centerx=(cx+oldx)/2, centery=(cy+oldy)/2;
                int radiusx=abs(cx-oldx), radiusy=abs(cy-oldy);
                fl_pie(centerx-radiusx, centery-radiusy+30, 2*radiusx, 2*radiusy, 0, 360);
            }
            break;
        case DM_HOVER:
            if (!current) current = bubbleAt(cx, cy);
            if (current == nullptr) return 1;
            current->setPosition(cx, cy);
            redraw();
            // todo not visible in draw
            current->draw(Bubble::OUTLINE);
            break;
        }
        return 1;
    }

    virtual int handle(int event) {
	int cx = Fl::event_x() - x();
	int cy = Fl::event_y() - y();

        switch(event) {
        case FL_MOVE:    return handle_hover  (event, cx, cy);
        case FL_PUSH:    return handle_press  (event, cx, cy);
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
    my_box->editmode = MyBox::DM_DRAW;
    my_box->submode = MyBox::DSM_RECT;
}

void cb_drawCirc(Fl_Widget* widget, void* data) {
    MyBox* my_box = static_cast<MyBox*>(data);
    assert( my_box != nullptr );
    mainWindow->cursor(FL_CURSOR_CROSS);
    my_box->editmode = MyBox::DM_DRAW;
    my_box->submode = MyBox::DSM_CIRC;
}

void cb_delShape(Fl_Widget* widget, void* data) {
    // TODO: a bit unintuitive maybe. should i change cursor shape and delete on click?
    if ( !current ) return;
    MyBox* my_box = static_cast<MyBox*>(data);
    assert( my_box != nullptr );
    my_box->comic->del( current );
    current = nullptr;
    my_box->redraw();
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

    Comicfile* comic = my_box->comic;

    // todo: write to file instead of cout
    std::cout << "write file to " << my_box->filename_out << std::endl;
    string ext = my_box->filename_out.substr(my_box->filename_out.rfind('.')+1);
    std::ofstream file_out(my_box->filename_out);
    if (!file_out.good()) {
        std::cerr << "could not open file '" << my_box->filename_out << "' for writing" << std::endl;
        exit(-1);
    }
    if      (ext ==  "xml") my_box->comic->writeXML(file_out);
    else if (ext == "json") my_box->comic->writeJSON(file_out);
    else {
        std::cerr << "cannot save to unknown output file format: " << ext << std::endl;
        exit(-1);
    }
}

int main(int argc, char **argv) {
    Comicfile::addFontpath("./fonts");
    Comicfile* comic;
    if (argc < 2 || argc > 3) {
	std::cerr<< "usage: "<< argv[0] <<" <XML/JSON file> [output file]\n";
	exit(-1);
    }
    comic = parse_file(argv[1]);
    comic->draw();

    // Create GUI
    mainWindow = new Fl_Window(imlib_image_get_width(), imlib_image_get_height()+30);
    MyBox* box = new MyBox(0,30);
    box->setComic(comic);
    box->filename_in = argv[1];
    if (argc == 3) {
        box->filename_out = argv[2];
    } else {
        // Derive filename and format for output file
        string filename_ext = box->filename_in.substr(box->filename_in.rfind('.')+1);
        box->filename_out = box->filename_in.substr(0, box->filename_in.rfind('.')+1) + comic->getLanguage() + "." + filename_ext;
    }

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

    // Draw all the bubbles
    for (const auto& b : bubbles) {
        b->draw(Bubble::ALL);
        if (b == current) {
            b->draw(Bubble::OUTLINE);
        }
    }
}
