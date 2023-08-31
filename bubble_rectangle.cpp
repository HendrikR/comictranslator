#include "bubble_rectangle.hpp"
#include <Imlib2.h>
//#include <stdlib.h>
//#include <math.h>
#include <iostream>
#include <stdio.h>

BubbleRectangle::BubbleRectangle(int _x0, int _y0, int _width, int _height,
				 CFont *_font, Color *_bgcolor)
    : Bubble(_font, _bgcolor, ""),
      x0(_x0), y0(_y0), width(_width), height(_height)
{
    font = _font;
    bgcolor = _bgcolor;
    if (width  < 8) std::cerr<< "Warning: Rectangle Width " <<width <<" too small.\n";
    if (height < 8) std::cerr<< "Warning: Rectangle Height "<<height<<" too small.\n";
}

bool BubbleRectangle::contains(int x, int y) const {
    if (x0 <= x && x < x0+width &&
	y0 <= y && y < y0+height) {
	return true;
    } else return false;
}

int BubbleRectangle::renderText() const {
    // Text schreiben
    string rest  = text;
    int fontsize = static_cast<int>(font->size);
    int txtheight = 0.8*fontsize;
    font->use();
    while(rest.length() > 0) {
	if (txtheight > height) {
	    std::cerr<< "Error: text does not fit, aborting: "<< text <<".\n";
	    break;
	}
	rest = drawTextLine(x0, y0+txtheight, rest, width, txtheight/(float)width, false);
	txtheight += fontsize*1.7;
    }
    return txtheight;
}


void BubbleRectangle::writeImage() const {
    bgcolor->use();
    imlib_image_fill_rectangle(x0, y0, width, height);
    this->renderText();
 }

void BubbleRectangle::writeXML(std::ostream& str) const {
    str << "<rectangle "
	<< "x0=\""<< x0 <<"\" "
	<< "y0=\""<< y0 <<"\" "
	<< "width=\""<< width <<"\" "
	<< "height=\""<< height <<"\" "
	<< "font=\"default\" "
	<< "bgcolor=\"default\">"
	<< text
	<< "</rectangle>\n";
}

void BubbleRectangle::writeJSON(std::ostream& str) const {
    str << "  {\"rectangle\": \n"
	<< "    \"x0\": " << x0 << ", "
	<< "    \"y0\": " << y0 << ", "
	<< "    \"width\": " << width << ", "
	<< "    \"height\": " << height << ", "
	<< "    \"font\": \"default\", "
	<< "    \"bgcolor\": \"default\", "
	<< "    \"text\": \"" << text << "\"},\n";
}

void BubbleRectangle::writeYAML(std::ostream& str) const {
    str << "  - rectangle "
	<< "    x0: " << x0 <<"\n"
	<< "    y0: " << y0 <<"\n"
	<< "    width: " << width <<"\n"
	<< "    height: " << height <<"\n"
	<< "    font: default\n"
	<< "    bgcolor: default\n"
	<< "    text: " << text << "\n";
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
    this->renderText();
}

void BubbleRectangle::setPosition(int x, int y) {
    x0 = x - width/2;
    y0 = y - height/2;
}
