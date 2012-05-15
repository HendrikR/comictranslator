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

using std::string;

#define BUFFSIZE        8192

char Buff[BUFFSIZE];

struct Color {
    int r,g,b,a;
    Color(int _r,int _g, int _b,int _a)
	: r(_r), g(_g), b(_b), a(_a) {};
    void use() {
	imlib_context_set_color(r,g,b,a);
    }
};

struct CFont {
    CFont(string _name, string _size, Color _color)
	: name(_name), size(_size), color(_color) {
	imlib_font = imlib_load_font((name + "/" + size).c_str());
    }
    ~CFont() {
	use();
	printf("freeing font\n");
	imlib_free_font();
    }
    string name;
    string size;
    Color color;
    Imlib_Font imlib_font;
    void use() {
	imlib_context_set_font(imlib_font);
	color.use();
    }
};

std::vector<Color> colors;
std::vector<CFont>  fonts;

static void XMLCALL xml_start(void *data, const char *el, const char **attr) {
    int i;
    std::map<string, string> args;
    for (i = 0; attr[i]; i += 2) {
	args[string(attr[i])] = string(attr[i+1]);
	//printf(" %s='%s'", attr[i], attr[i + 1]);
    }
    if (strcmp(el, "ellipse") == 0) {
	colors.back().use();
	imlib_image_fill_ellipse(atoi(args["centerx"].c_str()),
				 atoi(args["centery"].c_str()),
				 atoi(args["radiusb"].c_str())-4,
				 atoi(args["radiusa"].c_str())-4);
	fonts.back().use();
	imlib_text_draw(atoi(args["centerx"].c_str()),atoi(args["centery"].c_str())-6, "bla");
    } else if (strcmp(el, "rectangle") == 0) {
	// todo
    } else if (strcmp(el, "font") == 0) {
	// todo
	CFont fcurrent = CFont(args["name"],
			       args["size"],
			       Color(atoi(args["colorr"].c_str()),
				     atoi(args["colorg"].c_str()),
				     atoi(args["colorb"].c_str()),
				     255)
	    );
	fonts.push_back(fcurrent);
    } else if (strcmp(el, "bgcolor") == 0) {
	colors.push_back(Color(atoi(args["r"].c_str()),
			       atoi(args["g"].c_str()),
			       atoi(args["b"].c_str()),
			       atoi(args["a"].c_str())));
    } else if (strcmp(el, "comic") == 0) {
	Imlib_Image image = imlib_load_image("oots0123.gif");
	imlib_context_set_image(image);
    }
    //printf("%s", el);
    //printf("\n");
}

static void XMLCALL xml_end(void *data, const char *el) {
    if (strcmp(el, "bgcolor") == 0) {
	colors.pop_back();
    } else if (strcmp(el, "font") == 0) {
	fonts.pop_back();
    } else if (strcmp(el, "comic") == 0) {
	Imlib_Load_Error err;
	imlib_image_set_format("png");
	imlib_save_image_with_error_return("de0123.png", &err);
	if (err != 0) {
	    printf("error %d saving image.\n", err);
	}
	imlib_free_image();
    }
}

static void XMLCALL xml_data(void *userData, const XML_Char *s, int len) {
    // todo
}

int main(int argc, char *argv[]) {
    XML_Parser parser = XML_ParserCreate(NULL);
    if (! parser) {
	fprintf(stderr, "Couldn't allocate memory for parser\n");
	exit(-1);
    }

    XML_SetElementHandler(parser, xml_start, xml_end);
    XML_SetCharacterDataHandler(parser, xml_data);
    

    while(true) {
	int done;
	int len;

	len = (int)fread(Buff, 1, BUFFSIZE, stdin);
	if (ferror(stdin)) {
	    fprintf(stderr, "Read error\n");
	    exit(-1);
	}
	done = feof(stdin);

	if (XML_Parse(parser, Buff, len, done) == XML_STATUS_ERROR) {
	    fprintf(stderr, "Parse error at line %d: %s\n",
		    XML_GetCurrentLineNumber(parser),
		    XML_ErrorString(XML_GetErrorCode(parser)));
	    exit(-1);
	}

	if (done) break;
    }
    XML_ParserFree(parser);
    return 0;
}
