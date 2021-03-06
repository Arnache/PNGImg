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
  img.text_list.push_back(t1);

  img.addUTF8TextZ("Attempt", "à é À 🦄", "fr-fr", "Tentative");

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

  // by default, images are encoded in sRGB, below we modify this to a linear color space
  img.has_sRGB = false;
  img.has_gamma = true;
  img.decoding_gamma = 1.0 ;
  img.has_background = true;
  img.background_color.red = 0;
  img.background_color.green = 0;
  img.background_color.blue = 0;

  std::cout << "PNGImg.validate() returns ";
  int c=img.checks();
  if(c==PNGImg::CHECK_OK) std::cout << "OK";
  else std::cout << "BAD: " << c;
  std::cout << std::endl;

  img.save("test.png");
}
