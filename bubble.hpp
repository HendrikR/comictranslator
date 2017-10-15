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

    string replace_all(string str, string from, string to) const; // todo: this should go into a util library, or use some std::-function
    string drawTextLine(int x0, int y0, string text, int maxwidth, float rel_height, bool centered=true) const;
    Bubble(CFont* _font, Color* _bgcolor, string _text)
	: text(_text), font(_font), bgcolor(_bgcolor)
	{}
public:
    virtual void writeImage() const = 0;
    virtual void writeXML(std::ostream& str) const = 0;
    virtual void draw() const = 0;
    virtual bool contains(int x, int y) const = 0;
    void setText(string str);
};

class BubbleEllipse : public Bubble {
    int centerx, centery;
    int radiusx, radiusy;
public:
    BubbleEllipse(int _centerx, int _centery, int _radiusx, int _radiusy,
		  CFont *_font, Color *_bgcolor);
    virtual void writeImage() const override;
    virtual void writeXML(std::ostream& str) const override;
    virtual void draw() const override;
    virtual bool contains(int x, int y) const override;
    int ellipseWidth(float a, float b, float y) const;
};

class BubbleRectangle : public Bubble {
    int x0, y0;
    int width, height;
public:
    BubbleRectangle(int _x0, int _y0, int _width, int _height,
		    CFont *_font, Color *_bgcolor);
    virtual void writeImage() const override;
    virtual void writeXML(std::ostream& str) const override;
    virtual void draw() const override;
    virtual bool contains(int x, int y) const override;
};
#endif
