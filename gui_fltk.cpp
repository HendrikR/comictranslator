#include <FL/Fl.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_GIF_Image.H>
#include <vector>





typedef void (OverlayCallback)(void*);

class MyOverlayWindow : public Fl_Overlay_Window {
    std::vector<OverlayCallback*> oly_callbacks;    // array of child callbacks to draw overlays
    std::vector<void*> oly_data;                    // array of child userdata
public:
    MyOverlayWindow(int W,int H,const char*L=0):Fl_Overlay_Window(W,H,L) {
    }
    // SETUP AN OVERLAY DRAWING CALLBACK
    //     Called by child widgets to arrange callback to draw into overlay plane.
    //
    void addOverlayCallback(OverlayCallback *cb, void *data) {
        oly_callbacks.push_back(cb);        // add callback to array
        oly_data.push_back(data);           // add userdata to array
    }
    // INVOKE CHILDREN'S CALLBACKS TO DRAW OVERLAYS
    void draw_overlay() {
        // Invoke all the children's overlay callbacks
        for ( unsigned int t=0; t<oly_callbacks.size(); t++ ) {
            (*oly_callbacks[t])(oly_data[t]);
        }
    }
};

class MyCanvas : public Fl_Widget {
protected:
    MyOverlayWindow *ovl;
    Fl_Image* img;
    bool select_mode;
    int oldx, oldy;
public:
    MyCanvas(MyOverlayWindow* win, Fl_Image* _img)
	: Fl_Widget(0, 0, _img->w(), _img->h()),
	ovl(win),
	img(_img),
	select_mode(false)
    {
	ovl->addOverlayCallback(draw_ovl, (void*)this);
    }

    int handle(int e) {
        int ret = Fl_Widget::handle(e);
        switch ( e ) {
	case FL_ENTER:
	    ret = 1;                // FL_ENTER: must return(1) to receive FL_MOVE
	    break;
	case FL_PUSH:
	    if (Fl::event_button() == FL_LEFT_MOUSE) {
		select_mode = true;
		oldx = Fl::event_x();
		oldy = Fl::event_y();
	    }
	    break;
	case FL_RELEASE:
	    if (Fl::event_button() == FL_LEFT_MOUSE) {
		select_mode = false;
		damage(FL_DAMAGE_USER1);
	    }
	    break;
	case FL_DRAG:
	    if (select_mode) {
		damage(FL_DAMAGE_USER1);
		ret = 1; 
	    }
	    break;
        }
        return(1);
    }
    
    void draw() {
        if ( damage() != FL_DAMAGE_USER1 ) {
	    img->draw(x(), y(), w(), h());
            return;
        } else {
	    //ovl->redraw_overlay();
	    unsigned x0 = (oldx <= Fl::event_x()) ? oldx : Fl::event_x();
	    unsigned y0 = (oldy <= Fl::event_y()) ? oldy : Fl::event_y();
	    unsigned xs = abs(Fl::event_x() - oldx);
	    unsigned ys = abs(Fl::event_y() - oldy);
	    if (select_mode) {
		fl_color(FL_YELLOW);
		fl_rectf(x0, y0, xs, ys);
		//fl_arc(x0, y0, xs, ys, 0.0, 360.0);
	    } else {
		img->draw(x0, y0, xs, ys, x0, y0);
	    }
	}
    }
    static void draw_ovl(void* data) {
	MyCanvas* c = (MyCanvas*)data;
	unsigned x0 = (c->oldx <= Fl::event_x()) ? c->oldx : Fl::event_x();
	unsigned y0 = (c->oldy <= Fl::event_y()) ? c->oldy : Fl::event_y();
	unsigned xs = abs(Fl::event_x()-c->oldx);
	unsigned ys = abs(Fl::event_y()-c->oldy);
    }
};

Fl_Image* load_image(char* name) {
    Fl_Image* ret = 0;
    char* suffix = rindex(name, '.')+1;
    if (!strcasecmp(suffix, "jpg") || !strcasecmp(suffix, "jpeg")) {
	ret = new Fl_JPEG_Image(name);
    } else if (!strcasecmp(suffix, "png")) {
	ret = new Fl_PNG_Image(name);
    } else if (!strcasecmp(suffix, "gif")) {
	ret = new Fl_GIF_Image(name);
    } else if (!strcasecmp(suffix, "bmp")) {
	ret = new Fl_BMP_Image(name);
    }
    if (ret->w() == 0) {
	return 0;
    } else {
	return ret;
    }
}

int main() {
    MyOverlayWindow win(800, 600);
    win.resizable(win);
    Fl_Image* img = load_image("/home/hendrik/europe_1914_by_hillfighter-d362g5s.jpeg");
    if (img == 0) {
	fprintf(stderr, "Error: cannot load image file\n");
	exit(1);
    }
    Fl_Scroll scroll(0,0,800,600);
    MyCanvas b(&win, img);
    scroll.end();
    b.image(img);
    win.show();
    return(Fl::run());
}




#ifdef ignore
// Demonstrate using callbacks to draw into overlays -erco 8/23/06
#include <stdlib.h>
#include <vector>
using namespace std;
#include <FL/Fl.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>

typedef void (OverlayCallback)(void*);

class MyOverlayWindow : public Fl_Overlay_Window {
    vector<OverlayCallback*> oly_callbacks;    // array of child callbacks to draw overlays
    vector<void*> oly_data;                    // array of child userdata
public:
    MyOverlayWindow(int W,int H,const char*L=0):Fl_Overlay_Window(W,H,L) {
    }
    // SETUP AN OVERLAY DRAWING CALLBACK
    //     Called by child widgets to arrange callback to draw into overlay plane.
    //
    void AddOverlayCallback(OverlayCallback *cb, void *data) {
        oly_callbacks.push_back(cb);        // add callback to array
        oly_data.push_back(data);           // add userdata to array
    }
    // INVOKE CHILDREN'S CALLBACKS TO DRAW OVERLAYS
    void draw_overlay() {
        // Invoke all the children's overlay callbacks
        for ( unsigned int t=0; t<oly_callbacks.size(); t++ ) {
            (*oly_callbacks[t])(oly_data[t]);
        }
    }
};

class MyGroup : public Fl_Group {
    MyOverlayWindow *oly;

    // DRAW THE OVERLAY GRAPHIC FOR THIS INSTANCE OF CHILD
    static void OverlayCallback(void *data) {
        MyGroup *o = (MyGroup*)data;
        // Draw a random colored 'x'
        fl_color(rand() % 8);
        fl_line(o->x(), o->y(),        o->x()+o->w(), o->y()+o->h());
        fl_line(o->x(), o->y()+o->h(), o->x()+o->w(), o->y());
    }
public:
    MyGroup(MyOverlayWindow *win,int X,int Y,int W,int H,const char*L=0):Fl_Group(X,Y,W,H,L) {
        oly = win;
        win->AddOverlayCallback(OverlayCallback, (void*)this);
        end();
    }
    int handle(int e) {
        int ret = Fl_Group::handle(e);
        switch ( e ) {
	case FL_ENTER:
	    break;
	case FL_MOVE:
	    damage(FL_DAMAGE_USER1);
	    break;
        }
        return(1);
    }
    void draw() {
        Fl_Group::draw();
        //oly->redraw_overlay();          // tell parent to redraw child overlays
    }
};

int main(int argc, char **argv) {
    // Create overlay window
    MyOverlayWindow win(400,400);
    // Create four child groups, each with its own overlay graphic
    MyGroup g1(&win, 0,  0,  200,200);
    MyGroup g2(&win, 200,0,  200,200);
    MyGroup g3(&win, 0,  200,200,200);
    MyGroup g4(&win, 200,200,200,200);
    win.end();
    win.resizable(win);
    win.show();
    return(Fl::run());
}
#endif
