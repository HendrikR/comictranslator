#pragma once

#include "bubble.hpp"

class BubbleRectangle : public Bubble {
protected:
    int x0, y0;
    int width, height;
public:
    BubbleRectangle(int _x0, int _y0, int _width, int _height,
		    CFont *_font, Color *_bgcolor);
    virtual void writeImage() const override;
    virtual void writeXML(std::ostream& str, uint16_t indent = 0) const override;
    virtual void writeJSON(std::ostream& str, uint16_t indent = 0) const override;
    virtual void writeYAML(std::ostream& str, uint16_t indent = 0) const override;
    virtual void draw(DrawMode mode = ALL) const override;
    virtual int renderText() const override;
    virtual void setPosition(int x, int y) override;
    virtual bool contains(int x, int y) const override;
};

