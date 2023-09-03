#include "comicfile.hpp"
#include <expat.h>
#include <fstream>
#include <stdint.h>

const uint32_t BUFSIZE=0x10000;

Comicfile* comic;
Bubble* current_bubble;

int arg_i(std::map<string, string> &args, string name, int _default) {
    if (args.find(name) != args.end()) {
	return atoi(args[name].c_str());
    } else {
	return _default;
    }
}

float arg_f(std::map<string, string> &args, string name, float _default) {
    if (args.find(name) != args.end()) {
	return atof(args[name].c_str());
    } else {
	return _default;
    }
}
string arg_s(std::map<string, string> &args, string name, string _default) {
    if (args.find(name) != args.end()) {
	return args[name];
    } else {
	return _default;
    }
}

static void XMLCALL xml_start(void *data, const char *_elem, const char **attr) {
    string elem = string(_elem);
    // Argumente sammeln
    std::map<string, string> args;
    for (int i = 0; attr[i]; i += 2) {
	args[string(attr[i])] = string(attr[i+1]);
    }

    if (elem == "ellipse") {
	CFont* font = comic->getFont(arg_s(args, "font", "default"));
	Color* bgcolor = comic->getColor(arg_s(args, "bgcolor", "default"));
	int centerx = arg_i(args, "centerx"),  centery = arg_i(args, "centery");
	int radiusx = arg_i(args, "radiusx"),  radiusy = arg_i(args, "radiusy");
	current_bubble = new BubbleEllipse(centerx, centery, radiusx, radiusy,
					   font, bgcolor);
    } else if (elem == "rectangle") {
	CFont* font = comic->getFont(arg_s(args, "font", "default"));
	Color* bgcolor = comic->getColor(arg_s(args, "bgcolor", "default"));
	int x0    = arg_i(args, "x0"),    y0 = arg_i(args, "y0");
	int width = arg_i(args, "width"), height = arg_i(args, "height");
	current_bubble = new BubbleRectangle(x0, y0, width, height,
					     font, bgcolor);
    } else if (elem == "font") {
	comic->add(arg_s(args, "id"),
		   new CFont(arg_s(args, "name"),
			     arg_f(args, "size", 8.0),
			     Color(arg_i(args, "colorr", 0),
				   arg_i(args, "colorg", 0),
				   arg_i(args, "colorb", 0),
				   arg_i(args, "colora", 255)
				 )));
    } else if (elem == "bgcolor") {
	comic->add(arg_s(args, "id"),
		   new Color(arg_i(args, "r", 255),
			     arg_i(args, "g", 255),
			     arg_i(args, "b", 255),
			     arg_i(args, "a", 255)));
    } else if (elem == "comic") {
	comic = new Comicfile(arg_s(args, "name"),
			      arg_s(args, "lang"));
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
	current_bubble->text += text;
    }
}

Comicfile* parse_XML(char* filename) {
    // Open & check XML file
    std::ifstream file_in(filename);
    if (!file_in.good()) {
	std::cerr<< "Error loading XML file "<< filename <<"\n";
	exit(-1);
    }
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
    file_in.close();
    XML_ParserFree(parser);
    return comic;
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
    str << "<bgcolor id=\"default\" r=\"255\" g=\"255\" b=\"255\" />\n";
    str << "<font id=\"default\" name=\"ComicSansMSBold\" size=\"8\" colorr=\"0\" colorg=\"0\" colorb=\"0\" />\n";

    for (const Bubble* b : bubbles) {
        b->writeXML( str );
    }
    str << "</comic>\n";

}

void Comicfile::writeJSON(std::ostream& str) const {
    str << "{'comicfile':[\n";
    str << " {'name':'" << imgfile << "',\n";
    str << "  'lang':'de',\n";
    str << "  'bgcolor':{'id':'default','r':255, 'g':255, 'b':255}\n";
    str << "  'font':{'id':'default', 'name':'ComicSansMSBold', 'size':8, 'colorr':0, 'colorg':0, 'colorb':0}\n";
    str << "  'bubbles':{\n";
    for (const Bubble* b : bubbles) {
        b->writeXML( str );
    }
    str << "}]}\n";
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
        b->writeXML( str );
    }
}

void Comicfile::addFontpath(string path) {
    CFont::addFontpath(path);
}
