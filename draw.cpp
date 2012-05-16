/*****************************************************************
 * draw.cpp
 *
 * Copyright 2012, Hendrik Radke
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the license contained in the
 * COPYING file that comes with the expat distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Draw the comic with the help of the XML file.
 */


#include <stdio.h>
#include <expat.h>
#include <magic.h>
#include <Imlib2.h>
#include <map>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <math.h>

using std::string;

#define BUFSIZE 4096

// Handling Colors and Fonts
struct Color {
    int r,g,b,a;

    Color(int _r,int _g, int _b,int _a)
	: r(_r), g(_g), b(_b), a(_a) {};

    void use() {
	imlib_context_set_color(r,g,b,a);
    }
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
	    fprintf(stderr, "Error loading font %s\n", name.c_str());
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

struct Bubble {
    string shape;
    std::map<string, string> args;
    string text;
    
    Bubble(string _shape, std::map<string, string> _args)
	: shape(_shape), args(_args), text() {};
    void appendText(string _text) {
	text += _text;
    }
    void draw();
};

std::vector<Color> colors;
std::vector<CFont>  fonts;

// Filename for the translated comic
string filename_out;

Bubble *current_bubble;

int arg_i(std::map<string, string> &args, string name, int _default=0) {
    if (args.find(name) != args.end()) {
	return atoi(args[name].c_str());
    } else {
	return _default;
    }
}

float arg_f(std::map<string, string> &args, string name, float _default=0.0) {
    if (args.find(name) != args.end()) {
	return atof(args[name].c_str());
    } else {
	return _default;
    }
}
string arg_s(std::map<string, string> &args, string name, string _default="") {
    if (args.find(name) != args.end()) {
	return args[name];
    } else {
	return _default;
    }
}

static void XMLCALL xml_start(void *data, const char *el, const char **attr) {
    // Argumente sammeln
    std::map<string, string> args;
    for (int i = 0; attr[i]; i += 2) {
	args[string(attr[i])] = string(attr[i+1]);
    }
    if (strcmp(el, "ellipse") == 0 || strcmp(el, "rectangle") == 0) {
	current_bubble = new Bubble(el, args);
    } else if (strcmp(el, "font") == 0) {
	fonts.push_back(CFont(arg_s(args, "name"),
			      arg_s(args, "size"),
			      Color(arg_i(args, "colorr", 0),
				    arg_i(args, "colorg", 0),
				    arg_i(args, "colorb", 0),
				    arg_i(args, "colora", 255)
				  )));
    } else if (strcmp(el, "bgcolor") == 0) {
	colors.push_back(Color(arg_i(args, "r", 255),
			       arg_i(args, "g", 255),
			       arg_i(args, "b", 255),
			       arg_i(args, "a", 255)));
    } else if (strcmp(el, "comic") == 0) {
	string filename_in = string(arg_s(args, "name"));
	Imlib_Load_Error errno;
	Imlib_Image image = imlib_load_image_with_error_return(arg_s(args, "name").c_str(), &errno);
	if (errno != 0) {
	    fprintf(stderr, "Error loading image '%s'\n", arg_s(args, "name").c_str());
	    exit(-1);
	}
	imlib_context_set_image(image);
	string filename_ext = filename_in.substr(filename_in.rfind('.')+1);
	imlib_image_set_format(filename_ext.c_str());
	filename_out = filename_in.substr(0, filename_in.rfind('.')+1) + arg_s(args, "lang", "en") + "." + filename_ext;
	imlib_context_set_image(image);
    }
}

static void XMLCALL xml_end(void *data, const char *el) {
    if (strcmp(el, "bgcolor") == 0) {
	colors.pop_back();
    } else if (strcmp(el, "font") == 0) {
	fonts.back().free();
	fonts.pop_back();
    } else if (strcmp(el, "comic") == 0) {
	Imlib_Load_Error err;
	imlib_save_image_with_error_return(filename_out.c_str(), &err);
	if (err != 0) {
	    fprintf(stderr, "error saving image %s.\n", filename_out.c_str());
	} else {
	    fprintf(stderr, "Writing translated file %s\n", filename_out.c_str());
	}
	imlib_free_image();
    } else if (strcmp(el, "ellipse") == 0 || strcmp(el, "rectangle") == 0) {
	assert(current_bubble != 0);
	current_bubble->draw();
	delete current_bubble;
	current_bubble = 0;
    }
}

static void XMLCALL xml_data(void *userData, const XML_Char *s, int len) {
    if (current_bubble) {
	// chomp
	while (s[0] == '\n' && len > 0) { s += 1; len -= 1; }
	while (s[len-1] == '\n' && len > 0) { len -= 1; }
	string text = string(s, len);
	current_bubble->appendText(text);
    }
}

int main(int argc, char *argv[]) {
    std::ifstream file_in;

    if(argc == 2) {
	file_in.open(argv[1]);
	if (!file_in.good()) {
	    fprintf(stderr, "Error loading XML file %s\n", argv[1]);
	    exit(-1);
	}
    } else {
	fprintf(stderr, "usage: %s <XML file>\n", argv[0]);
	exit(-1);
    }

    XML_Parser parser = XML_ParserCreate(NULL);
    if (! parser) {
	fprintf(stderr, "Couldn't allocate memory for parser\n");
	exit(-1);
    }

    XML_SetElementHandler(parser, xml_start, xml_end);
    XML_SetCharacterDataHandler(parser, xml_data);
    
    char Buff[BUFSIZE];
    while(!file_in.eof()) {
	file_in.read(Buff, BUFSIZE);
	
	if (XML_Parse(parser, Buff, strlen(Buff), file_in.eof()?1:0) == XML_STATUS_ERROR) {
	  fprintf(stderr, "Parse error at line %d: %s\n", XML_GetCurrentLineNumber(parser), XML_ErrorString(XML_GetErrorCode(parser)));
	  exit(-1);
	}
    }
    file_in.close();
    XML_ParserFree(parser);
    exit(0);
}

int ellipseWidth(float a, float b, float y) {
    assert(b <= a);    // OBdA sei b <= a.
    // Ellipsen-Gleichung x^2/a^2 - y^2/b^2 = 1, umgestellt nach x
    return static_cast<int>(round(sqrt((1.0 - (y*y)/(b*b)) * (a*a))*2.0));
}

string replace_all(string str, string from, string to) {
    string::size_type pos = 0;
    while( (pos = str.find(from, pos)) != string::npos) {
	str.replace(pos, from.size(), to);
	pos++;
    }
    return str;
}

string drawTextLine(int x0, int y0, string text, int maxwidth, float rel_height) {
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
    imlib_text_draw(x0+(maxwidth-width)/2 + 2,
		    y0 - height/2 - static_cast<int>(round(1.1*rel_height*(height/2.0))),
		    replace_all(oldprefix, "~", " ").c_str());
    //imlib_image_draw_line(x0, y0, x0+maxwidth, y0, 0);
    if (oldprefix == text) return "";
    else                   return text.substr(oldprefix.length()+1);
}

void Bubble::draw() {
    if (shape == "ellipse") {
	colors.back().use();
	int centerx = arg_i(args, "centerx"),  centery = arg_i(args, "centery");
	int radiusx = arg_i(args, "radiusx"),  radiusy = arg_i(args, "radiusy");
	assert(centerx >= radiusx);
	assert(centery >= radiusy);
	assert(radiusx >= 6 && radiusy >= 6);
	imlib_image_fill_ellipse(centerx, centery, radiusx-1, radiusy-1);
	fonts.back().use();
	int len      = text.length();
	int fontsize = static_cast<int>(round(atof(fonts.back().size.c_str())));
	// Wähle die Höhe der 1. Zeile so, dass das 1. Wort gerade hineinpasst.
	int width, height;
	int first_idx     = text.find(' ');
	string first_word = (first_idx != string::npos) ? text.substr(0,first_idx) : text;
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
		fprintf(stderr, "warning: text does not fit, aborting: %s.\n", text.c_str());
		break;
	    }
	    width = ellipseWidth(radiusx, radiusy, abs(height));
	    rest = drawTextLine(centerx-width/2, centery+height, rest, width, height/(float)radiusx);
	    height += fontsize*1.7;
	}
    } else if (shape == "rectangle") {
	// todo
    } else {
	fprintf(stderr, "Error: unknown bubble shape %s\n", shape.c_str());
    }
}

