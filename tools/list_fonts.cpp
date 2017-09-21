#include <Imlib2.h>
#include <iostream>


int main(int argc, char** argv) {
  for(int i=1; i<argc; i++) {
    imlib_add_path_to_font_path( argv[i] );
  }

  int num_paths, num_fonts;

  char** paths = imlib_list_font_path(&num_paths);
  char** fonts = imlib_list_fonts(&num_fonts);

  std::cout << num_paths <<" font paths:\n";
  for( int i=0; i<num_paths; i++) {
    std::cout << paths[i] << std::endl;
  }
  std::cout << "\n";

  std::cout << num_fonts << " fonts:\n";
  for( int i=0; i<num_fonts; i++) {
    std::cout << fonts[i] << std::endl;
  }
}
