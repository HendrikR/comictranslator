#ifndef _BUBBLE_H_
#define _BUBBLE_H_

#include "color.hpp"
#include "cfont.hpp"


class Bubble {
    //string shape;
    //std::map<string, string> args;
protected:
public:
    string text;
    CFont *font;
    Color *bgcolor;
    
    string replace_all(string str, string from, string to);
    string drawTextLine(int x0, int y0, string text, int maxwidth, float rel_height, bool centered=true);
public:
    virtual void writeImage() = 0;
    virtual void draw() = 0;
    virtual bool contains(int x, int y) = 0;
    void setText(string str);
};

class BubbleEllipse : public Bubble {
    int centerx, centery;
    int radiusx, radiusy;
public:
    BubbleEllipse(int _centerx, int _centery, int _radiusx, int _radiusy,
		  CFont *_font, Color *_bgcolor);
    virtual void writeImage();
    virtual void draw();
    virtual bool contains(int x, int y);
    int ellipseWidth(float a, float b, float y);
};

class BubbleRectangle : public Bubble {
    int x0, y0;
    int width, height;
public:
    BubbleRectangle(int _x0, int _y0, int _width, int _height,
		    CFont *_font, Color *_bgcolor);
    virtual void writeImage();
    virtual void draw();
    virtual bool contains(int x, int y);
};
#endif
