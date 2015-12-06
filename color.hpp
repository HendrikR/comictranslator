#ifndef _COLOR_H_
#define _COLOR_H_

struct Color {
    int r,g,b,a;

public:
    Color(int gray = 0, int alpha = 255);
    Color(int _r,int _g, int _b,int _a);
    void use();
};

#endif
