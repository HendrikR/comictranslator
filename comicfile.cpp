#include "comicfile.h"
#include <expat.h>
#include <fstream>

#define BUFSIZE 4096

// Filename for the translated comic
string filename_out;

Bubble *current_bubble;

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

    if (elem == "ellipse" || elem == "rectangle") {
	current_bubble = new Bubble(elem, args, fonts.back(), colors.back());
    } else if (elem == "font") {
	fonts.push_back(CFont(arg_s(args, "name"),
			      arg_s(args, "size", "8"),
			      Color(arg_i(args, "colorr", 0),
				    arg_i(args, "colorg", 0),
				    arg_i(args, "colorb", 0),
				    arg_i(args, "colora", 255)
				  )));
    } else if (elem == "bgcolor") {
	colors.push_back(Color(arg_i(args, "r", 255),
			       arg_i(args, "g", 255),
			       arg_i(args, "b", 255),
			       arg_i(args, "a", 255)));
    } else if (elem == "comic") {
	string filename_in = string(arg_s(args, "name"));
	Imlib_Load_Error errno;
	Imlib_Image image = imlib_load_image_with_error_return(arg_s(args, "name").c_str(), &errno);
	if (errno != 0) {
	    std::cerr<< "Error loading image '"<< arg_s(args, "name") <<"'\n";
	    exit(-1);
	}
	imlib_context_set_image(image);
	string filename_ext = filename_in.substr(filename_in.rfind('.')+1);
	imlib_image_set_format(filename_ext.c_str());
	filename_out = filename_in.substr(0, filename_in.rfind('.')+1) + arg_s(args, "lang", "en") + "." + filename_ext;
	imlib_context_set_image(image);
    }
}

static void XMLCALL xml_end(void *data, const char *_elem) {
    string elem = string(_elem);
    if (elem == "bgcolor") {
	colors.pop_back();
    } else if (elem == "font") {
	fonts.back().free();
	fonts.pop_back();
    } else if (elem == "comic") {
	Imlib_Load_Error err;
	imlib_save_image_with_error_return(filename_out.c_str(), &err);
	if (err != 0) {
	    std::cerr<< "error saving image "<< filename_out <<".\n";
	} else {
	    std::cerr<< "Writing translated file "<< filename_out <<"\n";
	}
	imlib_free_image();
    } else if (elem == "ellipse" || elem == "rectangle") {
	if (current_bubble == 0) {
	    std::cerr<< "Error: "<< elem <<" closed without opening tag.\n";
	    exit(-1);
	}
	current_bubble->draw();
	delete current_bubble;
	current_bubble = 0;
    }
}

static void XMLCALL xml_data(void *userData, const XML_Char *s, int len) {
    if (current_bubble) {
	// chomp
	while (s[0] == '\n' && len > 0) { s += 1; len -= 1; }
	while (s[len-1] == '\n' && len > 0) { len -= 1; }
	string text = string(s, len);
	current_bubble->appendText(text);
    }
}

void parse_XML(char* filename) {
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
}
