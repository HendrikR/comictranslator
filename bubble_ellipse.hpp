#pragma once

#include "bubble.hpp"

class BubbleEllipse : public Bubble {
protected:
    int centerx, centery;
    int radiusx, radiusy;
public:
    BubbleEllipse(int _centerx, int _centery, int _radiusx, int _radiusy,
		  CFont *_font, Color *_bgcolor);
    virtual void writeImage() const override;
    virtual void writeXML(std::ostream& str) const override;
    virtual void writeJSON(std::ostream& str) const override;
    virtual void writeYAML(std::ostream& str) const override;
    virtual void draw(DrawMode mode = ALL) const override;
    virtual bool contains(int x, int y) const override;
    virtual int renderText() const override;
    virtual void setPosition(int x, int y) override;
    int ellipseWidth(float a, float b, float y) const;
};
