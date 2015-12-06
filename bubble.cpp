#include "bubble.hpp"
#include <Imlib2.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

#include <stdio.h>

string Bubble::replace_all(string str, string from, string to) {
    string::size_type pos = 0;
    while( (pos = str.find(from, pos)) != string::npos) {
	str.replace(pos, from.size(), to);
	pos++;
    }
    return str;
}

string Bubble::drawTextLine(int x0, int y0, string text, int maxwidth, float rel_height, bool centered) {
    string prefix = "", oldprefix = "";
    int height, width = 0;
    while (width < maxwidth && oldprefix != text) {
	oldprefix = prefix;
	// Find the next word boundary
	size_t len = prefix.length();
	size_t word_idx = text.find(" ", len+1);
	
	if (word_idx != string::npos) {
	    prefix = text.substr(0, word_idx);
	} else {
	    prefix = text;
	}
	imlib_get_text_size(prefix.c_str(), &width, &height);
    }
    
    imlib_get_text_size(oldprefix.c_str(), &width, &height);
    int xpos = x0 + 2;
    if (centered) xpos += (maxwidth-width)/2;
    int ypos = y0 - height/2 - static_cast<int>(round(1.1*rel_height*(height/2.0)));
    imlib_text_draw(xpos, ypos, replace_all(oldprefix, "~", " ").c_str());
    //imlib_image_draw_line(x0, y0, x0+maxwidth, y0, 0);
    if (oldprefix == text) return "";
    else                   return text.substr(oldprefix.length()+1);
}

void Bubble::setText(string str) {
    text = str;
}

BubbleEllipse::BubbleEllipse(int _centerx, int _centery, int _radiusx, int _radiusy,
				  CFont *_font, Color *_bgcolor)
    : centerx(_centerx), centery(_centery), radiusx(_radiusx), radiusy(_radiusy)
{
    if (radiusx > centerx || radiusy > centery)
	std::cerr<< "Warning: Ellipse Radii ("<< radiusx <<", "<< radiusy <<")"
		 << " > Midpoint ("<< centerx <<", "<< centery <<")\n";
    if (radiusx < 8) std::cerr<< "Warning: Ellipse x-Radius "<<radiusx<<" too small.\n";
    if (radiusy < 8) std::cerr<< "Warning: Ellipse y-Radius "<<radiusy<<" too small.\n";
    font = _font;
    bgcolor = _bgcolor;
}

int BubbleEllipse::ellipseWidth(float a, float b, float y) {
    // Ellipsen-Gleichung x^2/a^2 - y^2/b^2 = 1, umgestellt nach x
    return static_cast<int>(round(sqrt((1.0 - (y*y)/(b*b)) * (a*a))*2.0));
}

bool BubbleEllipse::contains(int x, int y) {
    /*if (centerx-radiusx/2 <= x && x < centerx+radiusx/2 &&
	centery-radiusy/2 <= y && y < centery+radiusy/2) {
	return true;
    } else return false;*/
    x -= centerx;
    y -= centery;
    if ( ((x*x) / (float)(radiusx*radiusx)  +  (y*y) / (float)(radiusy*radiusy))  <= 1.0) {
	return true;
    } else return false;
    
}

void BubbleEllipse::writeImage() {
    int fontsize = static_cast<int>(font->size);//todo
    bgcolor->use();
    imlib_image_fill_ellipse(centerx, centery, radiusx-1, radiusy-1);
    
    // Wähle die Höhe der 1. Zeile so, dass das 1. Wort gerade hineinpasst.
    int width, height;
    int first_idx     = text.find(' ');
    string first_word = (first_idx != string::npos) ? text.substr(0,first_idx) : text;
    font->use();
    imlib_get_text_size(first_word.c_str(), &width, &height);
    height = -radiusy;
    do {
	height++;
	//printf("%s of width %d does not fit into width %d, increase h to %d\n",
	//first_word.c_str(), width, ellipseWidth(radiusx, radiusy, abs(height)), height);
    } while (ellipseWidth(radiusx, radiusy, abs(height)) < width && height <= 0);
    height += fontsize/3;
    
    // Schreibe den Text nacheinander in die Zeilen.
    string rest  = text;
    while(rest.length() > 0) {
	if (height > radiusy-2) {
	    std::cerr<< "Error: text does not fit, aborting: "<< text <<".\n";
	    break;
	}
	width = ellipseWidth(radiusx, radiusy, abs(height));
	rest = drawTextLine(centerx-width/2, centery+height, rest, width, height/(float)radiusx);
	height += fontsize*1.7;
    }
}

BubbleRectangle::BubbleRectangle(int _x0, int _y0, int _width, int _height,
				 CFont *_font, Color *_bgcolor)
    : x0(_x0), y0(_y0), width(_width), height(_height)
{
    font = _font;
    bgcolor = _bgcolor;
    if (width  < 8) std::cerr<< "Warning: Rectangle Width " <<width <<" too small.\n";
    if (height < 8) std::cerr<< "Warning: Rectangle Height "<<height<<" too small.\n";
}

bool BubbleRectangle::contains(int x, int y) {
    if (x0 <= x && x < x0+width &&
	y0 <= y && y < y0+height) {
	return true;
    } else return false;
}

void BubbleRectangle::writeImage() {
    int fontsize = static_cast<int>(font->size);//todo
    bgcolor->use();
    imlib_image_fill_rectangle(x0, y0, width, height);
    // Text schreiben
    string rest  = text;
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
}
