#ifndef _COMICFILE_H_
#define _COMICFILE_H_

#include <Imlib2.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <stdlib.h>
#include <math.h>

using std::string;


// Handling Colors and Fonts
struct Color {
    int r,g,b,a;

    Color(int _r,int _g, int _b,int _a)
	: r(_r), g(_g), b(_b), a(_a) {};

    void use() {
	imlib_context_set_color(r,g,b,a);
    }
    /*void parse(string str) {
	str = str.substr(1,-1);
	}*/
};

struct CFont {
    string name;
    string size;
    Color color;
    Imlib_Font imlib_font;

    CFont(string _name, string _size, Color _color)
	: name(_name), size(_size), color(_color) {
	imlib_font = imlib_load_font((name + "/" + size).c_str());
	if (imlib_font == NULL) {
	    std::cerr<< "Error loading font " <<name<< "\n";
	    exit(-1);
	}
    }

    void free() {
	imlib_context_set_font(imlib_font);
	imlib_free_font();
    }

    void use() {
	imlib_context_set_font(imlib_font);
	color.use();
    }
};

class Bubble {
    string shape;
    std::map<string, string> args;
    string text;
    CFont font;
    Color textcolor;
    Color bgcolor;
    
public:
    Bubble(string _shape, std::map<string, string> _args, CFont _font, Color _bgcolor)
	: shape(_shape), args(_args), text(), font(_font), bgcolor(_bgcolor) {}
    
    void appendText(string _text) {
	text += _text;
    }
    void draw();
};

static std::vector<Color> colors;
static std::vector<CFont>  fonts;

void parse_XML(char* filename);
int    arg_i(std::map<string, string> &args, string name, int    _default=0);
float  arg_f(std::map<string, string> &args, string name, float  _default=0.0);
string arg_s(std::map<string, string> &args, string name, string _default="");

#endif
