#include "color.hpp"
#include <Imlib2.h>

Color::Color(int gray, int alpha)
    : r(gray), g(gray), b(gray), a(alpha) {};
Color::Color(int _r,int _g, int _b,int _a)
    : r(_r), g(_g), b(_b), a(_a) {};

void Color::use() {
    imlib_context_set_color(r,g,b,a);
}
