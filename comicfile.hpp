#ifndef _COMICFILE_H_
#define _COMICFILE_H_

#include <Imlib2.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <math.h>
#include "color.hpp"
#include "cfont.hpp"
#include "bubble.hpp"
#include "bubble_ellipse.hpp"
#include "bubble_rectangle.hpp"

using std::string;


class Comicfile {
    // TODO: add a "name" field to the comicfile, to write out to XML?
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
    void del(Bubble* b) {
        auto b_in = std::find(bubbles.begin(), bubbles.end(), b);
        if (b_in != bubbles.end()) bubbles.erase(b_in);
    }
    Color*  getColor(string id) {
	return colors[id];
    }
    CFont*  getFont(string id) {
	return fonts[id];
    }
    void writeImage() const;
    void writeXML(std::ostream& str) const;
    void draw() const;
    static void addFontpath(string path = "./fonts");
};


Comicfile* parse_XML(char* filename);
int    arg_i(std::map<string, string> &args, string name, int    _default=0);
float  arg_f(std::map<string, string> &args, string name, float  _default=0.0);
string arg_s(std::map<string, string> &args, string name, string _default="");

#endif
