#ifndef _CFONT_H_
#define _CFONT_H_

#include <string>
#include <Imlib2.h>
#include "color.hpp"
using std::string;

struct CFont {
    string name;
    float size;
    Color color;
    Imlib_Font imlib_font;

public:
    CFont(string _name, float _size, Color _color);
    void free();
    void use();
    static void addFontpath(string path);
};

#endif
