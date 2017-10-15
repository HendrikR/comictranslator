/*****************************************************************
 * draw.cpp
 *
 * Copyright 2012-2014, Hendrik Radke
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the license contained in the
 * COPYING file.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Fill text bubbles of a comic with text, according to an XML
 * file as generated by detect.cpp.
 */

#include "comicfile.hpp"
#include <fstream>

int main(int argc, char *argv[]) {
    Comicfile::addFontpath("./fonts");
    if(argc == 2) {
	Comicfile* comic = parse_XML(argv[1]);
	comic->writeImage();
    } else {
	std::cerr<< "usage: "<< argv[0] <<" <XML file>\n";
	exit(-1);
    }
    exit(0);
}

void Comicfile::draw() const {}
void BubbleEllipse::draw() const {}
void BubbleRectangle::draw() const {}
