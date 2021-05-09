# PNGImg

## Description

*PNGImg* is a simple C++ class for saving/loading png images, with support of textual comments in the metadata.

Its interface basically has function members load() and a save() and settings are set via variable members.

There is no image transformation/filtering or whatsoever. The PNG image is decoded to/encoded from a memory buffer\* which is in raw format. *It is the user's task to interpret this raw format according to the PNG specification.*  
\*: in this implementation, a C++ vector  
It is also the user's responsibility to send valid settings and data to PNGImg.

## Disclaimer

PNGImg is provided "as is" with absolutely no guarrantee of any sort.

## Example

Computer generate an image and save it in a linear color space (gamma=1) together with a textual comment.

```c++
#include "PNGImg.cpp"

int main() {

  int width=300;
  int height=200;

  PNGImg img(width,height,8,PNG_COLOR_TYPE_RGB);

  // assign one text comment
  PNGImg::pngText t1;
  t1.type = PNG_TEXT_COMPRESSION_zTXt;
  t1.key="Comment";
  t1.text="Coucou ;)";
  img.text.push_back(t1);

  // the size of img.data is not set by img's constructor, so we set it here
  img.data.resize(width*height*3);

  // fill img.data with computer generated values
  int k;
  for(int li=0; li<height; li++) {
    for(int co=0; co<width; co++) {
      k=3*(width*li+co);
      img.data[k+0] = li; // red
      img.data[k+1] = co; // green
      img.data[k+2] = li+co; // blue
    }
  }

  // by default, images are encoded in sRGB; below we modify this to a linear color space
  img.has_sRGB = false;
  img.has_gamma = true;
  img.decoding_gamma = 1.0 ;

  img.saveImage("test.png");
}
```
