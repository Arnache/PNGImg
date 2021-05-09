#include <iostream>
#include "PNGImg.cpp"

int main(int argc, char** argv) {

  PNGImg img;

  std::cout << "Endianness: " << (img.big_endian ? "big" : "little") << std::endl;

  if(argc!=2) { 
    std::cout << "please call with the filename as the only parameter." << std::endl;
    return 0;
  }

  char *filename = argv[1];

  std::cout << "Loading begins of file " << filename << std::endl;

  int code = img.load(filename);

  std::cout << "Loading finished with return code " << code << std::endl;

  if(code != 0) {
    std::cerr << "Loading failed" << std::endl;
    return 1;
  }

  std::cout << "PNGImg.validate() returns ";
  int c=img.checks();
  if(c==PNGImg::CHECK_OK) std::cout << "OK";
  else std::cout << "BAD: " << c;
  std::cout << std::endl;

  std::cout << "(width,heigt) = (" << img.width << "," << img.height << ")" << std::endl;

  std::cout << "PNGImg.has_sRGB = " << (img.has_sRGB ? "true" : "false") << std::endl;

  std::cout << "PNGImg.has_gamma = " << (img.has_gamma ? "true" : "false") << std::endl;
  std::cout << "PNGImg.decoding_gamma = " << img.decoding_gamma << std::endl;

  std::cout << "Thanks for your attention." << std::endl;
}
