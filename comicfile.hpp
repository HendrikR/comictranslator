#ifndef _COMICFILE_H_
#define _COMICFILE_H_

#include <Imlib2.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include "color.hpp"
#include "cfont.hpp"
#include "bubble.hpp"

using std::string;


class Comicfile {
    string imgfile;
    string language;
    std::map<string, Color*> colors;
    std::map<string, CFont*> fonts;
public:
    std::vector<Bubble*> bubbles;
    Comicfile(string _filename, string _language)
	: imgfile(_filename), language(_language) {}
    Color*  add(string id, Color* c) {
	colors[id] = c;
	return c;
    }
    CFont* add(string id, CFont* f) {
	fonts[id] = f;
	return f;
    }
    Bubble* add(Bubble* b) {
	bubbles.push_back(b);
	return b;
    }
    Color*  getColor(string id) {
	return colors[id];
    }
    CFont*  getFont(string id) {
	return fonts[id];
    }
    void writeImage();
    void draw();
};


Comicfile* parse_XML(char* filename);
int    arg_i(std::map<string, string> &args, string name, int    _default=0);
float  arg_f(std::map<string, string> &args, string name, float  _default=0.0);
string arg_s(std::map<string, string> &args, string name, string _default="");

#endif
