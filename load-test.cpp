#include <iostream>
#include <string>
#include "PNGImg.cpp"

std::string bool_to_text(bool b) {
  if(b) return "true";
  return "false";
}

std::string color16_to_text(png_color_16 color, int type) {
  std::string s;
  switch(type) {
    case PNG_COLOR_TYPE_PALETTE:
      s += "index: "+std::to_string(color.index);
      break;
    case PNG_COLOR_TYPE_RGB: case PNG_COLOR_TYPE_RGBA:
      s += "R: "+std::to_string(color.red)+", G: "+std::to_string(color.green)+", B: "+std::to_string(color.blue);
      break;
    case PNG_COLOR_TYPE_GRAY: case PNG_COLOR_TYPE_GRAY_ALPHA:
      s += "gray: "+std::to_string(color.gray);
      break;
    default: s += "error";
  }
  return s;
}

std::string color_type_to_text(int type) {
  switch(type) {
    case PNG_COLOR_TYPE_PALETTE : return "paletted";
    case PNG_COLOR_TYPE_RGB : return "truecolor";
    case PNG_COLOR_TYPE_RGBA : return "truecolor+alpha";
    case PNG_COLOR_TYPE_GRAY : return "grayscale";
    case PNG_COLOR_TYPE_GRAY_ALPHA : return "grayscale+alpha";
  }
  return std::string("invalid color type code ") +std::to_string(type);
}

std::string sRGB_intent_to_text(int code) {
  switch(code) {
    case PNG_sRGB_INTENT_PERCEPTUAL : return "perceptual";
    case PNG_sRGB_INTENT_RELATIVE : return "relative";
    case PNG_sRGB_INTENT_SATURATION : return "saturation";
    case PNG_sRGB_INTENT_ABSOLUTE : return "absolute";
  }
  return std::string("invalid intent code ") +std::to_string(code);
}

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

  std::cout << "Loading finished with return code " << code << (code==0 ? " (good)" : " (error, see PNGImg's manual for the meaning of this error code)") << std::endl;

  if(code != 0) {
    std::cerr << "Loading failed" << std::endl;
    return 1;
  }

  std::cout << "PNGImg.validate() returns ";
  int c=img.checks();
  if(c==PNGImg::CHECK_OK) std::cout << "OK";
  else std::cout << "BAD: " << c;
  std::cout << std::endl;

  std::cout << "image type: " << color_type_to_text(img.color_type) << std::endl;

  std::cout << "width, height = " << img.width << ", " << img.height << "" << std::endl;

  std::cout << "PNGImg.has_sRGB = " << bool_to_text(img.has_sRGB) << std::endl;
  if(img.has_sRGB) {
    std::cout << "PNGImg.sRGB_intent = " << sRGB_intent_to_text(img.sRGB_intent) << std::endl;
  }

  std::cout << "PNGImg.has_gamma = " << (img.has_gamma ? "true" : "false") << std::endl;
  if(img.has_gamma) {
    std::cout << "PNGImg.decoding_gamma = " << img.decoding_gamma << std::endl;
  }

  std::cout << "PNGImg.has_background = " << (img.has_background ? "true" : "false") << std::endl;
  if(img.has_background) {
    std::cout << "PNGImg.background_color = " << color16_to_text(img.background_color,img.color_type) << std::endl;
  }

  std::cout << "PNGImg.has_tRNS = " << (img.has_tRNS ? "true" : "false") << std::endl;
  if(img.has_tRNS) {
    if(img.color_type==PNG_COLOR_TYPE_PALETTE) {
      std::cout << "PNGImg.transparency_palette.size() = " << img.transparency_palette.size() << std::endl;
    }
    else {
      std::cout << "PNGImg.transparent_color = " << color16_to_text(img.transparent_color,img.color_type) << std::endl;
    }
  }

  std::cout << "Thanks for your attention." << std::endl;
}
