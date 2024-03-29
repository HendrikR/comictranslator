#pragma once

#include "color.hpp"
#include "cfont.hpp"


class Bubble {
    //string shape;
    //std::map<string, string> args;
public:
    enum DrawMode { OUTLINE, FILL, ALL };
protected:
    string text;
    CFont *font;
    Color *bgcolor;

    string replace_all(string str, string from, string to) const; // todo: this should go into a util library, or use some std::-function
    string drawTextLine(int x0, int y0, string text, int maxwidth, float rel_height, bool centered=true) const;
public:
    Bubble(CFont* _font, Color* _bgcolor, string _text)
	: text(_text), font(_font), bgcolor(_bgcolor)
	{}
    virtual void writeImage() const = 0;
    virtual void writeXML(std::ostream& str, uint16_t indent = 0) const = 0;
    virtual void writeJSON(std::ostream& str, uint16_t indent = 0) const = 0;
    virtual void writeYAML(std::ostream& str, uint16_t indent = 0) const = 0;
    virtual void draw(DrawMode mode = ALL) const = 0;
    virtual bool contains(int x, int y) const = 0;
    virtual int renderText() const = 0;
    virtual void setPosition(int x, int y) = 0;
    //virtual std::pair<int, int> getPosition() const = 0;
    void setText(string str);
    std::string getText() const;
};
