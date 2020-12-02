#include "bubble_ellipse.hpp"
#include <Imlib2.h>
#include <cmath>
#include <iostream>

#include <stdio.h>

BubbleEllipse::BubbleEllipse(int _centerx, int _centery, int _radiusx, int _radiusy,
				  CFont *_font, Color *_bgcolor)
    : Bubble(_font, _bgcolor, ""),
      centerx(_centerx), centery(_centery), radiusx(_radiusx), radiusy(_radiusy)
{
    if (radiusx > centerx || radiusy > centery)
	std::cerr<< "Warning: Ellipse Radii ("<< radiusx <<", "<< radiusy <<")"
		 << " > Midpoint ("<< centerx <<", "<< centery <<")\n";
    if (radiusx < 8) std::cerr<< "Warning: Ellipse x-Radius "<<radiusx<<" too small.\n";
    if (radiusy < 8) std::cerr<< "Warning: Ellipse y-Radius "<<radiusy<<" too small.\n";
    font = _font;
    bgcolor = _bgcolor;
    text = "";
}

int BubbleEllipse::ellipseWidth(float a, float b, float y) const {
    // Ellipsen-Gleichung x^2/a^2 - y^2/b^2 = 1, umgestellt nach x
    return static_cast<int>(round(sqrt((1.0 - (y*y)/(b*b)) * (a*a))*2.0));
}

bool BubbleEllipse::contains(int x, int y) const {
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

void BubbleEllipse::writeImage() const {
    int fontsize = static_cast<int>(font->size);//todo
    bgcolor->use();
    imlib_image_fill_ellipse(centerx, centery, radiusx-1, radiusy-1);

    // Wähle die Höhe der 1. Zeile so, dass das 1. Wort gerade hineinpasst.
    int width, height;
    unsigned first_idx     = text.find(' ');
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
	    std::cerr<< "Error: text does not fit: "<< text <<"\n";
	    break;
	}
	width = ellipseWidth(radiusx, radiusy, abs(height));
	rest = drawTextLine(centerx-width/2, centery+height, rest, width, height/(float)radiusx);
	height += fontsize*1.7;
    }
}

void BubbleEllipse::writeXML(std::ostream& str) const {
    str << "<ellipse "
	<< "centerx=\""<< centerx <<"\" "
	<< "centery=\""<< centery <<"\" "
	<< "radiusx=\""<< radiusx <<"\" "
	<< "radiusy=\""<< radiusy <<"\" "
	<< "font=\"default\" "
	<< "bgcolor=\"default\">"
	<< text
	<< "</ellipse>\n";
}

void BubbleEllipse::writeJSON(std::ostream& str) const {
    str << "  {\"ellipse\": \n"
	<< "    \"centerx\": " << centerx << ", "
	<< "    \"centery\": " << centery << ", "
	<< "    \"radiusx\": " << radiusx << ", "
	<< "    \"radiusy\": " << radiusy << ", "
	<< "    \"font\": \"default\", "
	<< "    \"bgcolor\": \"default\", "
	<< "    \"text\": \"" << text << "\"},\n";
}

void BubbleEllipse::writeYAML(std::ostream& str) const {
    str << "  - ellipse\n"
	<< "    centerx: " << centerx << "\n"
	<< "    centery: " << centery << "\n"
	<< "    radiusx: " << radiusx << "\n"
	<< "    radiusy: " << radiusy << "\n"
	<< "    font: default\n"
	<< "    bgcolor: default\n"
	<< "    text: " << text << "\n";
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
