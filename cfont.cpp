#include "cfont.hpp"
#include "color.hpp"
#include <Imlib2.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>

CFont::CFont(string _name, float _size, Color _color)
    : name(_name), size(_size), color(_color) {
    std::ostringstream ss;
    ss << size;
    string s_size = string(ss.str());
    imlib_font = imlib_load_font((name + "/" + s_size).c_str());
    if (imlib_font == NULL) {
	std::cerr<< "Error loading font " <<name<< "\n";
	exit(-1);
    }
}

void CFont::free() {
    imlib_context_set_font(imlib_font);
    imlib_free_font();
}

void CFont::use() {
    imlib_context_set_font(imlib_font);
    color.use();
}
