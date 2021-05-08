# Manual

**PNGImg** is a simple low level C++ class to load/save PNG images, with some support for text metadata.

## Disclaimer

PNGImg is provided "as is" with absolutely no guarrantee of any sort.

## Licence

CC-By-SA Arnaud Chéritat.

## PNG version

The version of PNG has been 1.2 for a long time and is not likely to change in the near future.

The *PNG specification* can be found at [http://www.libpng.org/pub/png/spec/iso/index-object.html](http://www.libpng.org/pub/png/spec/iso/index-object.html)

## Image type

The current version of PNG has 5 image types:

libpng constant name | type
--- | ---
PNG_COLOR_TYPE_PALETTE | Paletted
PNG_COLOR_TYPE_RGB | True color
PNG_COLOR_TYPE_RGBA | True color with alpha
PNG_COLOR_TYPE_GRAY| Grayscale
PNG_COLOR_TYPE_GRAY_ALPHA | Grayscale with alpha

where *alpha* means alpha channel, means there is an opacity value for each pixel.

## Channels and bit depth

Most images have been encoded on 8 bit deep channels but there are more and more use of deeper encodings.
 
*Channels* can be Red (R), Green (G), Blue (B), Alpha (A) and Gray, and are encoded with a certain number n of bits, allowing for instance values from 0 to 255 with 8 bits.

PNG has from the start been designed for up to 16 bits deep channels. Here is a table of allowed bit depths for each image type :

type | allowed channel bit depths
--- | ---
..._PALETTE | *special*
..._RGB | 8, 16
..._RGBA | 8, 16
..._GRAY| 1, 2, 4, 8, 16
..._GRAY_ALPHA | 8, 16

For non-paletted types the bit detph is given by the variable member `bit_depth`.

For paletted images, the variable `bit_depth` has another use: the palette and transparency palette can hold up to 256 values. If they are sufficiently small, a value of `bit_depth` of n = 4, 2 or 1 can be provided to indicate that the color index take value at most 2^n - 1 = 15, 3 or 1. (This allows *packing* of 2, 4 or 8 pixels in a single byte; however, and even though libpng allows dealing with such packed raw formats, *we do not support* this feature in PNGImg.)

## Raw format

As a C++ class, PNGImg has a variable member called `data` which is of type `vector<char>` and is designed to hold the data in raw format.

The raw format is very simple and depends on the image type and bit depth: scan each line from top to bottom. For each line scan from left to right. For each pixel, lay the channels (or pixel index for paletted images) as bytes successively as indicated below, where each letter is a byte, the leftmost first, and where a repeated letter means the two bytes of a 16 bits unsigned integer (in big-endian or litte-endian, see below).

image type | `bit_depth` | byte layout for one pixel
--- | --- | ---
..._PALETTE | 1, 2, 4, 8 | I
..._RGB | 8 | RGB
..._RGB | 16 | RRGGBB
..._RGBA | 8 | RGBA
..._RGBA | 16 | RRGGBBAA
..._GRAY| 1, 2, 4, 8 | G
..._GRAY| 16 | GG
..._GRAY_ALPHA | 8 | GA
..._GRAY_ALPHA | 16 | GGAA

## Endianness of 16 bits per channel images

If you only use images with at most 8 bits per channel (i.e. R,G,B,Gray,Alpha channels take values from 0 to at most 255, i.e. encoded on one byte) then you do not need to read this paragraph. Otherwise, if your want to save a 16 bits per channel image, the constructor of PNGImg determines the endianness of your system and stores it in the boolean `PNGImg.big_endian`. The data will be (de/en)coded to/from the memory according to this bool. You are allowed to change its value if you want to invert/enforce an endianness (prior to the call of Load() or Save(); they will --not-- modify the value of the bool).

## Support of text comments

The PNG formal allows for insertion of textual comments in the metadata. The encoding of the string can be either latin-1 or utf-8 and it can be stored/read in/from the png either directly or (/de)compressed by libpng. The combitation of these types are:  

libpng constant name | value | chunk type storing it
--- | --- | ---
PNG_TEXT_COMPRESSION_NONE | -1 | tEXt
PNG_TEXT_COMPRESSION_zTXt | 0 | zTXt
PNG_ITXT_COMPRESSION_NONE | 1 | iTXt
PNG_ITXT_COMPRESSION_zTXt | 2 | iTXt

PNGImg supports load/save of those comments, they are stored in the variable member `text` a `vector` of `pngText`, the latter being an imitation of png_text but easier to use.

## Assumptions

The compilation must be done with at least C++11.
We assume that on your compiler, a `char` is one byte long (8 bits).
We do not make an assumption on endianness of the system.
The code has only been tested on a few systems so there may be other implicit assumptions that the author is not aware of.

### Notes

This manual is in progress, below you'll see a bunch of notes that will eventually be better organized.

- Saving:
  - the member variables `width`, `height`, `bit_depth`, `color_type`, (also `interlace_type`, `compression_type`, `filter_method`) must be correct, coherent, and in accordance with `data.size()`
  - as a matter of fact, in the current version of PNG (1.2),
     compression type and filter method can only take one value so
     just don't touch them
  - `Save()` will fail (I'm not sure of the exact consequences) if
     the data vector is too small
- supports load/save of ancillary chunks sRGB, gAMA, tRNS, tEXt, zTXt and iTXt
- --No-- planned support for iCCP in the short term
- --No-- planned support for sPLT, oFFs, pHYs, sCAl, bKGD, tIME, hIST, sBIT
- since we `#include <png.h>` we get all their #defines, essentially a lot of all-caps constants, which unfortunately cannot be wrapped in a namespace afaik.

### Motives

I have been programming fractal sets drawing short programs in C++ for a while, saving the resulting images into PNGs. I have been using the ability of PNG to hold textual comments in their metadata (tEXT chunks) to also save the algorithm parameters the program was called with (like: where did I zoom on the Mandelbrot set?). 

The C library *libpng* is the canonical way to encode/decode PNG images. However it quite complicated to use. Resource management (avoiding memory leaks, crashes, etc) requires a good deal of attention. C++ has mechanisms for reducing these risks (for instance RAII) and for shorter overall* syntax (via classes).  
\* Despite the verbosisty induced by the namespace and template mechanisms.

## Version notes

### v0.4

modernized C++ code

- memory leaks: got rid of all 'new' and 'delete', resource managed the RAII way, (except there remains one malloc)
- tried to use std::vector<> whenever possible
- problem identified, to be treated in a future release: the transparency palette has to be dealt with for paletted images

### v0.41

- continued to modernize C++ code
- added tRNS support
- allowed endianness swap also on load (using the same variable member `big_endian`)
- detected a memory leak : non-freed malloc (contradicting the doc of PNG 1.4.0)

### v0.42

- added zTXt and iTXt support
- renamed `compression` to `type` in pngText
- included description of the raw format
- found why the malloc was not freed: libpng 1.6.0 deprecated the freeing system explained in the doc of 1.4.0, fixed the problem by replacing the malloc by a const_cast'ed ref to the vector 
- as a nice side effect, there is no more malloc

### future releases:

- deal with a finer error reporting, in particular report non-fatal errors after successful Load() of a non-conforming PNG
