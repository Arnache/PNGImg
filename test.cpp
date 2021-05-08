#include <iostream>
#include "PNGImg.cpp"

int main() {

  int width=300;
  int height=200;

  PNGImg img(width,height,8,PNG_COLOR_TYPE_RGB);

  std::cout << "Endianness: " << (img.big_endian ? "big" : "little") << std::endl;

  PNGImg::pngText t1;
  t1.type = PNG_TEXT_COMPRESSION_zTXt;
  t1.key="Comment";
  t1.text="Coucou ;)";
  img.text.push_back(t1);

  img.data.resize(width*height*3);

  int k;
  for(int li=0; li<height; li++) {
    for(int co=0; co<width; co++) {
      k=3*(width*li+co);
      img.data[k+0] = li; // red
      img.data[k+1] = co; // green
      img.data[k+2] = li+co; // blue
    }
  }

  std::cout << "PNGImg.validate() returns ";
  int c=img.checks();
  if(c==PNGImg::CHECK_OK) std::cout << "OK";
  else std::cout << "BAD: " << c;
  std::cout << std::endl;
  std::cout << "PNGImg.has_sRGB = " << (img.has_sRGB ? "true" : "false") << std::endl;

  img.saveImage("test.png");
}
