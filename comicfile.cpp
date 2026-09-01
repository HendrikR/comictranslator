#include "comicfile.hpp"
#include <expat.h>
#include <fstream>
#include <sstream>
#include <stdint.h>
#include "jute/jute.h"

const uint32_t BUFSIZE=0x10000;

Comicfile* comic;
Bubble* current_bubble;

int xarg_i(const std::map<string, string> &args, const string& name, int _default=0) {
    return args.find(name) != args.end() ? atoi(args.at(name).c_str()) : _default;
}
float xarg_f(const std::map<string, string> &args, const string& name, float _default=0.0) {
    return args.find(name) != args.end() ? atof(args.at(name).c_str()) : _default;
}
string xarg_s(const std::map<string, string> &args, const string& name, string _default="") {
    return args.find(name) != args.end() ? args.at(name) : _default;
}

int jarg_i(jute::jValue obj, const string& name, int _default=0) {
    return obj[name].get_type() != jute::JUNKNOWN ? obj[name].as_int() : _default;
}
int jarg_d(jute::jValue obj, const string& name, double _default=0.0) {
    return obj[name].get_type() != jute::JUNKNOWN ? obj[name].as_double() : _default;
}
string jarg_s(jute::jValue obj, const string& name, string _default="") {
    return obj[name].get_type() != jute::JUNKNOWN ? obj[name].as_string() : _default;
}

static void XMLCALL xml_start(void *data, const char *_elem, const char **attr) {
    string elem = string(_elem);
    // Argumente sammeln
    std::map<string, string> args;
    for (int i = 0; attr[i]; i += 2) {
	args[string(attr[i])] = string(attr[i+1]);
    }

    if (elem == "ellipse") {
	CFont* font = comic->getFont(xarg_s(args, "font", "default"));
	Color* bgcolor = comic->getColor(xarg_s(args, "bgcolor", "default"));
	int centerx = xarg_i(args, "centerx"),  centery = xarg_i(args, "centery");
	int radiusx = xarg_i(args, "radiusx"),  radiusy = xarg_i(args, "radiusy");
	current_bubble = new BubbleEllipse(centerx, centery, radiusx, radiusy,
					   font, bgcolor);
    } else if (elem == "rectangle") {
	CFont* font = comic->getFont(xarg_s(args, "font", "default"));
	Color* bgcolor = comic->getColor(xarg_s(args, "bgcolor", "default"));
	int x0    = xarg_i(args, "x0"),    y0 = xarg_i(args, "y0");
	int width = xarg_i(args, "width"), height = xarg_i(args, "height");
	current_bubble = new BubbleRectangle(x0, y0, width, height,
					     font, bgcolor);
    } else if (elem == "font") {
	comic->add(xarg_s(args, "id"),
		   new CFont(xarg_s(args, "name"),
			     xarg_f(args, "size", 8.0),
			     Color(xarg_i(args, "colorr", 0),
				   xarg_i(args, "colorg", 0),
				   xarg_i(args, "colorb", 0),
				   xarg_i(args, "colora", 255)
				 )));
    } else if (elem == "bgcolor") {
	comic->add(xarg_s(args, "id"),
		   new Color(xarg_i(args, "r", 255),
			     xarg_i(args, "g", 255),
			     xarg_i(args, "b", 255),
			     xarg_i(args, "a", 255)));
    } else if (elem == "comic") {
	comic = new Comicfile(xarg_s(args, "name"),
			      xarg_s(args, "lang"));
    }
}

static void XMLCALL xml_end(void *data, const char *_elem) {
    string elem = string(_elem);
    if (elem == "bgcolor") {
	//colors.pop_back();
    } else if (elem == "comic") {
    } else if (elem == "ellipse" || elem == "rectangle") {
	if (current_bubble == 0) {
	    std::cerr<< "Error: "<< elem <<" closed without opening tag.\n";
	    exit(-1);
	}
	comic->add(current_bubble);
	current_bubble = nullptr;
    }
}

static void XMLCALL xml_data(void *userData, const XML_Char *s, int len) {
    if (current_bubble) {
	// chomp
	while (s[0] == '\n' && len > 0) { s += 1; len -= 1; }
	while (s[len-1] == '\n' && len > 0) { len -= 1; }
	string text = string(s, len);
	current_bubble->setText(current_bubble->getText() + text);
    }
}


Comicfile* parse_file(const string& filename) {
    // Open & check file
    std::ifstream file_in(filename);
    if (!file_in.good()) {
        std::cerr<< "Error loading XML file "<< filename << std::endl;
        exit(-1);
    }
    string extension = filename.substr(filename.find_last_of(".") + 1);
    for (auto& c : extension) c = tolower(c);
    if (extension == "xml") {
        return Comicfile::readXML(file_in);
    } else if (extension == "json") {
        return Comicfile::readJSON(file_in);
    } else {
        std::cerr<< "Unknown file format: "<< extension << std::endl;
        exit(-1);
    }
}

Comicfile* Comicfile::readXML(std::istream& file_in) {
    // Create Parser
    XML_Parser parser = XML_ParserCreate(NULL);
    if (! parser) {
	std::cerr<< "Couldn't allocate memory for parser\n";
	exit(-1);
    }

    XML_SetElementHandler(parser, xml_start, xml_end);
    XML_SetCharacterDataHandler(parser, xml_data);

    char Buff[BUFSIZE];
    while(!file_in.eof()) {
	file_in.read(Buff, BUFSIZE);
	if (XML_Parse(parser, Buff, string(Buff).length(), file_in.eof()?1:0) == XML_STATUS_ERROR) {
	    std::cerr<< "Parse error at line "
		     << XML_GetCurrentLineNumber(parser) <<": "
		     << XML_ErrorString(XML_GetErrorCode(parser)) << "\n";
	    exit(-1);
	}
    }
    XML_ParserFree(parser);
    return comic;
}

Comicfile* Comicfile::readJSON(std::istream& file_in) {
    // TODO: json files can only have one comic per file, xml files can have more. Which is more sensible?
    Comicfile* cf = new Comicfile("", "");
    std::stringstream file_sstream;
    file_sstream << file_in.rdbuf();
    jute::jValue json = jute::parser::parse(file_sstream.str());
    //assert(json.get_type() == jute::JOBJECT);
    // todo check existence of comicfile and bubbles
    cf->imgfile = jarg_s(json["comicfile"], "name", "");
    cf->language = jarg_s(json["comicfile"], "lang", "en");

    jute::jValue j_fonts = json["comicfile"]["fonts"];
    for (size_t i=0; i<j_fonts.size(); ++i) {
        cf->add(jarg_s(j_fonts[i], "id"),
                new CFont(jarg_s(j_fonts[i], "name"),
                          jarg_d(j_fonts[i], "size", 8.0),
                          Color(jarg_i(j_fonts[i], "colorr", 0),
                                jarg_i(j_fonts[i], "colorg", 0),
                                jarg_i(j_fonts[i], "colorb", 0),
                                jarg_i(j_fonts[i], "colora", 255))));
    }

    jute::jValue j_bgcolors = json["comicfile"]["bgcolors"];
    for (size_t i=0; i<j_bgcolors.size(); ++i) {
        cf->add(jarg_s(j_bgcolors[i], "id"),
                   new Color(jarg_i(j_bgcolors[i], "r", 255),
                             jarg_i(j_bgcolors[i], "g", 255),
                             jarg_i(j_bgcolors[i], "b", 255),
                             jarg_i(j_bgcolors[i], "a", 255)));
    }

    jute::jValue j_bubbles = json["comicfile"]["bubbles"];
    for (size_t i=0; i<j_bubbles.size(); ++i) {
        jute::jValue j_bubble = j_bubbles[i];
        // TODO: set to default if not present
        Bubble* bubble;
        CFont* font = cf->getFont(jarg_s(j_bubble, "font", "default"));
        Color* bgcolor = cf->getColor(jarg_s(j_bubble, "bgcolor", "default"));
        const std::string shape = jarg_s(j_bubble, "shape");
        if (shape == "ellipse") {
            int centerx = jarg_i(j_bubble, "centerx"),  centery = jarg_i(j_bubble, "centery");
            int radiusx = jarg_i(j_bubble, "radiusx"),  radiusy = jarg_i(j_bubble, "radiusy");
            bubble = new BubbleEllipse(centerx, centery, radiusx, radiusy, font, bgcolor);
        } else if (shape == "rectangle") {
            int x0    = jarg_i(j_bubble, "x0"),    y0 = jarg_i(j_bubble, "y0");
            int width = jarg_i(j_bubble, "width"), height = jarg_i(j_bubble, "height");
            bubble = new BubbleRectangle(x0, y0, width, height, font, bgcolor);
        } else {
            std::cerr<< "Unknown shape: '"<< shape <<"'\n";
            exit(-1);
        }
	bubble->setText(jarg_s(j_bubble, "text"));
        cf->bubbles.push_back(bubble);
    }
    return cf;
}

void Comicfile::writeImage() const {
    // Load the original image
    Imlib_Load_Error err;
    Imlib_Image image = imlib_load_image_with_error_return(imgfile.c_str(), &err);
    if (err != 0) {
	std::cerr<< "Error loading image '"<< imgfile <<"'\n";
	exit(-1);
    }
    imlib_context_set_image(image);

    // Derive filename and format for output file
    string filename_ext = imgfile.substr(imgfile.rfind('.')+1);
    imlib_image_set_format(filename_ext.c_str());
    string filename_out = imgfile.substr(0, imgfile.rfind('.')+1) + language + "." + filename_ext;
    imlib_context_set_image(image);

    // Draw all the bubbles
    for (const Bubble* b : bubbles) {
	b->writeImage();
    }

    // Save the new image
    imlib_save_image_with_error_return(filename_out.c_str(), &err);
    if (err != 0) {
	std::cerr<< "error saving image "<< filename_out <<".\n";
    } else {
	std::cout<< "Writing translated file "<< filename_out <<"\n";
    }
    imlib_free_image();
}

void Comicfile::writeXML(std::ostream& str) const {
    str << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    str << "<comic name=\"" << imgfile << "\" lang=\"de\">\n";
    // TODO: do this properly with comicfile->fonts/colors
    str << "<bgcolor id=\"default\" r=\"255\" g=\"255\" b=\"255\" />\n";
    str << "<font id=\"default\" name=\"ComicSansMSBold\" size=\"8\" colorr=\"0\" colorg=\"0\" colorb=\"0\" />\n";

    for (const Bubble* b : bubbles) {
        b->writeXML( str );
    }
    str << "</comic>\n";

}

void Comicfile::writeJSON(std::ostream& str) const {
    // TODO: do this properly with comicfile->fonts/colors
    str << "{\"comicfile\": {\n"
        << "    \"name\":\"" << imgfile << "\",";
    str << R"EOD(
    "lang":"de",
    "bgcolors": [
        {"id":"default","r":255, "g":255, "b":255}
    ],
    "fonts": [
        {"id":"default", "name":"ComicSansMSBold", "size":8, "colorr":0, "colorg":0, "colorb":0}
    ],
    "bubbles": [
)EOD";
    string delim = "";
    for (const Bubble* bubble : bubbles) {
        //if (&b != std::prev(bubbles.end())) str << delim;
        str << delim;
        bubble->writeJSON( str, 8);
        delim = ",\n";
    }
    str << "    ]}}\n";
}

void Comicfile::writeYAML(std::ostream& str) const {
    str << "comicfile\n";
    str << "  name: " << imgfile << "\n";
    str << "  lang: de\n";
    str << "  bgcolor:\n";
    str << "   id:default\n";
    str << "   r:255\n";
    str << "   g:255\n";
    str << "   b:255\n";
    str << "  font:\n";
    str << "   id: default\n";
    str << "   name: ComicSansMSBold\n";
    str << "   size: 8\n";
    str << "   colorr: 0\n";
    str << "   colorg: 0\n";
    str << "   colorb: 0\n";
    str << "  bubbles:\n";
    for (const Bubble* b : bubbles) {
        b->writeYAML( str );
    }
}

void Comicfile::addFontpath(const string& path) {
    CFont::addFontpath(path);
}
