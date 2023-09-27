#include "bubble.hpp"
#include <Imlib2.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <stdio.h>

string Bubble::replace_all(string str, string from, string to) const {
    string::size_type pos = 0;
    while( (pos = str.find(from, pos)) != string::npos) {
	str.replace(pos, from.size(), to);
	pos++;
    }
    return str;
}

string Bubble::drawTextLine(int x0, int y0, string text, int maxwidth, float rel_height, bool centered) const {
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

std::string Bubble::getText() const {
    return text;
}
