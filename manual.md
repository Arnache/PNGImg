# Manual

**PNGImg** is a simple low level C++ class to load/save PNG images, with some support for text metadata.

## Disclaimer

PNGImg is provided "as is" with absolutely no guarantee of any sort. In now way the author can be liable for any damage it may induce.

## Licence

CC BY-SA 4.0 Arnaud Chéritat.

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

For non-paletted types the bit depth is given by the variable member `bit_depth`.

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

If you only use images with at most 8 bits per channel (i.e. R,G,B,Gray,Alpha channels take values from 0 to at most 255, i.e. encoded on one byte) then you do not need to read this paragraph. Otherwise, if your want to save a 16 bits per channel image, the constructor of PNGImg determines the endianness of your system and stores it in the boolean `PNGImg.big_endian`. The data will be (de/en)coded to/from the memory according to this bool. You are allowed to change its value if you want to invert/enforce an endianness (prior to the call of load() or save(); they will --not-- modify the value of the bool).

## Support of text comments

The PNG formal allows for insertion of textual comments in the metadata. The encoding of the string can be either latin-1 or utf-8 and it can be stored/read in/from the png either raw or (/de)compressed by libpng. The combination of these types are:

libpng constant name | value | charset | compressed? | PNG chunk type
--- | --- | --- | --- | ---
PNG_TEXT_COMPRESSION_NONE | -1 | latin-1 | no | tEXt
PNG_TEXT_COMPRESSION_zTXt | 0 | latin-1 | yes | zTXt
PNG_ITXT_COMPRESSION_NONE | 1 | utf-8 | no | iTXt
PNG_ITXT_COMPRESSION_zTXt | 2 | utf-8 | yes | iTXt

PNGImg supports load/save of those comments, they are stored in the variable member `text`, a `vector<pngText>`, where `pngText` is a structure that imitates libpng's `png_text` but is, I hope, easier to use.

## Assumptions

The compilation must be done with at least C++11.
We assume that on your compiler, a `char` is one byte long (8 bits).
We do not make an assumption on endianness of the system.
The code has only been tested on a few systems so there may be other implicit assumptions that the author is not aware of.

## The PNGImg class

### Member variables:

#### Image type variables

- `width` and `height`: they are of type png_uint_32, which means unsigned with 32 bits (4 bytes) or -maybe more on some systems but the PNG file format does not allow sizes of 2^32 or more
- `int color_type`: explained in earlier sections, one of 5 possible types of PNG images
- `int bit_depth`: explained in earlier sections, should only take values 1, 2, 4, 8 or 16 depending on the color type
- `int interlace_type`: can take the values PNG_INTERLACE_NONE (default) or PNG_INTERLACE_ADAM7 (see the [PNG specification](http://www.libpng.org/pub/png/spec/iso/index-object.html))

The next two variables were introduced for possible future version of PNG, but as of 2021, they have not yet found a use,\* so *just leave them as they are*.

- `int compression_type` = must be PNG_COMPRESSION_TYPE_DEFAULT, which is its default value anyway
- `int filter_method` = must be PNG_FILTER_TYPE_DEFAULT, which is its default value anyway

\*: In the MNG file format, `filter_method` can in fact take another value, but PNGImg does not support MNG.

#### Image data related variables

- `std::vector<char> data`: the data will be stored here. When loading the vector is automatically resized to the correct value. When saving it is the user's responsibility to provide a vector that is wide enough.
- `bool big_endian` (default value: endianness of the system, determined at runtime in the constructor). Endianness of the raw format when `bit_depth==16` for the concerned image types. `load()` and `save()` do not modify this value,  they use it in the concerned case to decode/encode the PNG file into the raw format.
- `std::vector<png_color> palette` when saving a paletted image, this vector should have an allowed size (i.e. at least 1, at most 2^bit_depth)
- `bool has_transparent` (default: false). For paletted image, tells whether or not to use the `transparency_palette`. For rgb and gray (without alpha), tells whether or not to use the `transparent_color` below.
- `std::vector<png_byte> transparency_palette` when saving, the size of this vector should be between 0 and the size of `palette`
- `png_color_16 transparent_color`
- `png_color_16 background_color`

The structures `png_color` and `png_color_16` are defined in `png.h`, as follows:

```c++
typedef struct png_color_struct
{
   png_byte red;
   png_byte green;
   png_byte blue;
} png_color;

typedef struct png_color_16_struct
{
   png_byte index;    /* used for palette files */
   png_uint_16 red;   /* for use in red green blue files */
   png_uint_16 green;
   png_uint_16 blue;
   png_uint_16 gray;  /* for use in grayscale files */
} png_color_16;
```

#### Member variables related to supported ancillary chunks

- `bool has_sRGB` (default: true). If true then supersedes gamma and an sRGB chunk will be included in the PNG telling the image format should be . According to the PNG specification we also save a cHRM and a gAMA chunk specific values (1.0/2.2 for gAMA: we ignore `decoding_gamma`), but only for compatibility with old decoders.
- `bool sRGB_intent` (default: PNG_sRGB_INTENT_PERCEPTUAL). Meaningful only if `has_sRGB==true`. If you do not know what to choose, use the default value.

    constant name | int value
    --- | ---
    PNG_sRGB_INTENT_PERCEPTUAL | 0
    PNG_sRGB_INTENT_RELATIVE  | 1
    PNG_sRGB_INTENT_SATURATION | 2
    PNG_sRGB_INTENT_ABSOLUTE   | 3

- `bool has_gamma` (default: false). Saving: if true and if `has_rRGB==false` then a gAMA chunk will be saved with value `1.0/decoding_gamma`. If false then a gAMA chunk will be saved if `has_sRGB` is true (see above). Loading: it will become true if either there was an sRGB chunk, in which case `decoding_gamma` will hold a value close to 2.2 (ignoring the gAMA chunk value even if it is present: this is a consequence of libpng's behaviour), or there was not but there was a gAMA chunk, `decoding_gamma` then holds the inverse of its value.
- `double decoding_gamma` (default: 1.0). Saving: ignored if `has_sRGB==true`.
- `bool has_background` (default: false). Related to the bKGD chunk. See the [PNG specification](http://www.libpng.org/pub/png/spec/iso/index-object.html).
- `png_color_16 background_color`: If `has_background==true` then this holds the background color. See above for the definition of `png_color_16`.

- `std::vector<pngText> text` the comments, in the format described below (text is allowed to be empty)

```c++
  struct pngText {
                 // the comments below are copied and adapted from png.h
   int type = PNG_TEXT_COMPRESSION_NONE; 
                         /* value:
 PNG_TEXT_COMPRESSION_NONE (-1): tEXt, none
 PNG_TEXT_COMPRESSION_zTXt  (0): zTXt, deflate
 PNG_ITXT_COMPRESSION_NONE  (1): iTXt, none
 PNG_ITXT_COMPRESSION_zTXt  (2): iTXt, deflate 
                          */
   std::string key;      /* keyword, 1-79 character description,
                            Latin-1 encoded, may contain only printable 
                            characters (codes 32-126 and 161-255) */
   std::string text;     /* text, may be an empty string (ie "") encoded
                            in Latin-1 (tEXt and zTXt)
                            or in UTF-8 (iTXt) */
   // for iTXT only:
   std::string lang;     /* language code, 0-79 ASCII characters*/
   std::string lang_key; /* keyword translated UTF-8 string, 0 or more
                            chars */
  };  
```

### Member functions

- `int getPixelSize()`:
Returns the number of bytes taken by each pixel in raw format, according to the image type and bit depht indicated by member variables `color_type` and `bit_depth`. Will return 0 if the pair (color_type,bit_depth) does not take allowed values.
- `int checks(bool strict_data_size=false)`:
This convenience function does some checks (but not all possible checks)
about the coherence of the member variables
width, height, bit_depth, color_type, interlace_type, 
compression_type, filter_method, and the data.size().
More details are given in the comments just before the function definition, in `PNGImg.cc`.
- `int load(const char* fileName)` and `int load(FILE* file)`:
load a PNG file and store it in the PNGImg object. In case of failure, returns an error code, on success, returns 0.
- `int save(const char* fileName)` and `int save(FILE* file)`:
the opposite of load 😉. 
  - the member variables `width`, `height`, `bit_depth`, `color_type`, (also `interlace_type`, `compression_type`, `filter_method`) must be correct, coherent, and in accordance with `data.size()`
  - `save()` will fail (I'm not sure of the exact consequences) if the data vector is too small
  - In case of failure, returns an error code, on success, returns 0.

Error codes for `load()` and `save()`:

- INIT_FAILED=2
- FILE_OPEN_FAILED=3
- GET_HEADER_FAILED=4
- LIBPNG_ERROR=5
- PALETTE_MISSING=6


## Notes

- --No-- planned support for iCCP in the short term
- --No-- planned support for sPLT, oFFs, pHYs, sCAl, tIME, hIST, sBIT
- --No-- support of progressive display of image.
- since we `#include <png.h>` we get all their #defines, essentially a lot of all-caps constants, which unfortunately cannot be wrapped in a namespace afaik.

## Motives

I have been programming fractal sets drawing short programs in C++ for a while, saving the resulting images into PNGs. I have been using the ability of PNG to hold textual comments in their metadata (tEXT chunks) to also save the algorithm parameters the program was called with (like: where did I zoom on the Mandelbrot set?). 

The C library *libpng* is the canonical way to encode/decode PNG images. However it quite complicated to use. Resource management (avoiding memory leaks, crashes, etc) requires a good deal of attention. C++ has mechanisms for reducing these risks (for instance RAII) and for shorter overall* syntax (via classes).  
\* Despite the verbosity induced by the namespace and template mechanisms.

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

### v0.43

- changed the names of the load and save functions
- described the PNGImg class member variables and functions
- corrected bug in save(const char* filename)
- corrected std::string(null) in load() (this is illegal, but was called when there are text chunks)

### v0.44

- added support of bKGD
- support of sRGB is now complete (added `sRGB_intent`) 
- I forgot to do `has_gamma=true;` in load when a gamma chunk is present; this is now corrected
- renamed `has_transparent` to `has_tRNS`

### v0.45

- in save and load replaced C-style string by std::string
- added four helper function to append each of the four types of texts in one line
- renamed member `text` to `text_list`

### future releases:

- deal with a finer error reporting, in particular report non-fatal errors after successful load() of a non-conforming PNG

### abandoned trails

- wanted: replace FILE* by a C++ equivalent like ifstream  
problem: libpng is designed to handle a FILE*
