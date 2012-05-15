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
#include <string>
#include <string.h>
#include <iostream>

using std::string;
using std::map;

#define BUFFSIZE        8192

char Buff[BUFFSIZE];

static void XMLCALL start(void *data, const char *el, const char **attr) {
    int i;
    map<string, string> args;
    for (i = 0; attr[i]; i += 2) {
	args[string(attr[i])] = string(attr[i+1]);
	//printf(" %s='%s'", attr[i], attr[i + 1]);
    }
    if (strcmp(el, "ellipse") == 0) {
	imlib_context_set_color(atoi(args["colorr"].c_str()),
				atoi(args["colorg"].c_str()),
				atoi(args["colorb"].c_str()),
				255);
	imlib_image_fill_ellipse(atoi(args["centerx"].c_str()),
				 atoi(args["centery"].c_str()),
				 atoi(args["radiusb"].c_str())-4,
				 atoi(args["radiusa"].c_str())-4);
	imlib_context_set_color(0, 0, 0, 255);
	imlib_text_draw(atoi(args["centerx"].c_str()),atoi(args["centery"].c_str())-6, "bla");
    }
    //printf("%s", el);
    //printf("\n");
}

static void XMLCALL end(void *data, const char *el) {
}

int main(int argc, char *argv[]) {
    XML_Parser p = XML_ParserCreate(NULL);
    if (! p) {
	fprintf(stderr, "Couldn't allocate memory for parser\n");
	exit(-1);
    }

    XML_SetElementHandler(p, start, end);

    Imlib_Image image = imlib_load_image("oots0123.gif");
    imlib_context_set_image(image);
    Imlib_Font font = imlib_load_font("FreeSans/8");
    imlib_context_set_font(font);

    while(true) {
	int done;
	int len;

	len = (int)fread(Buff, 1, BUFFSIZE, stdin);
	if (ferror(stdin)) {
	    fprintf(stderr, "Read error\n");
	    exit(-1);
	}
	done = feof(stdin);

	if (XML_Parse(p, Buff, len, done) == XML_STATUS_ERROR) {
	    fprintf(stderr, "Parse error at line %d: %s\n",
		    XML_GetCurrentLineNumber(p),
		    XML_ErrorString(XML_GetErrorCode(p)));
	    exit(-1);
	}

	if (done) break;
    }
    Imlib_Load_Error err;
    imlib_image_set_format("png");
    imlib_save_image_with_error_return("de0123.png", &err);
    printf("saving yields error %d/%d\n", err, IMLIB_LOAD_ERROR_NO_LOADER_FOR_FILE_FORMAT);
    imlib_free_font();
    imlib_free_image();

    XML_ParserFree(p);
    return 0;
}
