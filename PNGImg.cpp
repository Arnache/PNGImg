// PNGImg v0.45 (2021)
// Author: Arnaud Chéritat
// Licence: CC BY-SA 4.0

#include <png.h> // it itself includes many other C libraries, including stdio
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>

class PNGImg {

public:
 
  /*** nested classes ***/

  // An imitation of pnt_text but with char* replaced by std::string and
  // with text_length/itxt_length made redundant by text.size() 
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

  /*** Constants ***/

  // return values of the load/save functions
  static const int INIT_FAILED=2;
  static const int FILE_OPEN_FAILED=3;
  static const int GET_HEADER_FAILED=4;
  static const int LIBPNG_ERROR=5;
  static const int PALETTE_MISSING=6;

  // return value of the check function is 0 = CHECK_OK 
  // or a bitwise-or of the following flags:
  static const int 
    CHECK_OK = 0
  , ZERO_WIDTH_OR_HEIGHT = 1
  , BAD_INTERLACE = 1 << 1
  , BAD_COMPRESSION_TYPE = 1 << 2
  , BAD_FILTER_TYPE = 1 << 3
  , BAD_IMAGE_TYPE = 1 << 4
  , BAD_BIT_DEPTH = 1 << 5
  , BAD_DATA_SIZE = 1 << 6
  , BAD_PALETTE_SIZE = 1 << 7
  ;

  /*** Static functions ***/

  // a std::find function wrapper for a more readable code
  template<typename T, typename V> // V should be a vector<> of type comparable by == to T
  static bool in(T needle, V haystack) {
    return std::find(haystack.begin(), haystack.end(), needle)!= haystack.end(); 
  }

  /*** Variables ***/

  // note that some of the member variables are initialized and some aren't
 
  // required

  png_uint_32 width;
  png_uint_32 height;
  int bit_depth;
  int color_type;
  int interlace_type = PNG_INTERLACE_NONE;
  int compression_type = PNG_COMPRESSION_TYPE_DEFAULT;
  int filter_method = PNG_FILTER_TYPE_DEFAULT;

  std::vector<char> data; // when saving, this vector is allowed to be bigger than needed

  // required for paletted images

  std::vector<png_color> palette; // when saving a paletted image, this
    // vector should have an allowed size (i.e. at least 1, at most 2^bit_depth)
  bool has_tRNS=false;
    // for paletted image, tells whether or not to use the transparency palette
    // for rgb and gray (without alpha), tells whether or not to use the transparent_color
  std::vector<png_byte> transparency_palette; // when saving, there
    // should be between 0 and the pallette size, elements
  png_color_16 transparent_color;

  bool has_background=false;
  png_color_16 background_color;

  // required for images having 16 bit per channel

  png_uint_16 _one=1;
  bool big_endian = !(1 == *(unsigned char *)&(_one));
    // When C++20 becomes supported enough, we can start setting big_endian in a nicer way
    // bool big_endian = std::endian::native == std::endian::big

  // ancillary

  bool has_sRGB = true;
  int sRGB_intent = PNG_sRGB_INTENT_PERCEPTUAL;
  bool has_gamma = false;
  double decoding_gamma = 1.0; // sRGB = approx 2.2
  std::vector<pngText> text_list;

  /*** functions ***/

  /* default constructor does nothing
   * note that some member variables have default values, some don't
   */ 
  PNGImg() {
  }

  /* a weak constructor
   * does not initialize data, palette, 
   */ 
  PNGImg(png_uint_32 w, png_uint_32 h, int b, int c,
    int i=PNG_INTERLACE_NONE,
    int cm=PNG_COMPRESSION_TYPE_DEFAULT,
    int f=PNG_FILTER_TYPE_DEFAULT)
  {
    width=w;
    height=h;
    bit_depth=b;
    color_type=c;
    interlace_type=i;
    compression_type=cm;
    filter_method=f;
  }

  /* destructor */
  ~PNGImg() {
  }


  int getPixelSize() { // 0 means error, i.e the pair
    // (color_type,bit_depth) does not take allowed values 
    switch(color_type) {
      case PNG_COLOR_TYPE_RGB_ALPHA : {
        if(bit_depth==8)
          return 4;
        else if(bit_depth==16)
          return 8;
        else
          return 0;
      } break;
      case PNG_COLOR_TYPE_RGB : {
        if(bit_depth==8)
          return 3;
        else if(bit_depth==16)
          return 6;
        else
          return 0;
      } break;
      case PNG_COLOR_TYPE_PALETTE : {
        if(in(bit_depth, std::vector<int> {1, 2, 4, 8}))
          return 1;
        else
          return 0;
      } break;
      case PNG_COLOR_TYPE_GRAY_ALPHA : {
        if(bit_depth==8)
          return 2;
        else if(bit_depth==16)
          return 4;
        else
          return 0;
      } break;
      case PNG_COLOR_TYPE_GRAY : {
        if(in(bit_depth, std::vector<int> {1, 2, 4, 8}))
          return 1;
        else if(bit_depth==16)
          return 2;
        else
          return 0;
      } break;

      default: return 0;
    }
    return 0; // unreachable
  }

  /* This function does some checks (but not all possible checks)
   * about the coherence of the member variables
   * width, height, bit_depth, color_type, interlace_type, 
   * compression_type, filter_method, and the data.size().
   *
   * input: 
   * - strict_data_size: 
   *   - if false (default value), it will check that
   * data.size() <= minimal value; note that the data elements
   * with index beyond this minimal value will be unused
   *   - if true, it will check that data.size() == minimal value
   *
   * return value:
   * 0 if all checks succeed
   * bitwise or of failed checks flags
   *
   * Notes: 
   * - This function is provided for the convenience of the user of 
   * PNGImg, it is not used in the code of PNGImg.
   * - Returning 0 (CHECK_OK) is "correlated" to being able to 
   * successfully Save() the image. Here "correlated" means that we 
   * did not (yet) check/complete the test suite.
   * - After a successful Load(), the image will always have check()
   * return CHECK_OK (otherwise, there is a bug in libpng or in PNGImg).
   * - We do not store the result: we leave responsibility of using a
   * valid img to the user of PNGImg.
   */
  int checks(bool strict_data_size=false) {
    int answer = 0;

    if(width==0 || height==0) answer |= ZERO_WIDTH_OR_HEIGHT;

    if(!(interlace_type==PNG_INTERLACE_NONE || interlace_type==PNG_INTERLACE_ADAM7)) answer |= BAD_INTERLACE;

    if(compression_type!=PNG_COMPRESSION_TYPE_DEFAULT) answer |= BAD_COMPRESSION_TYPE;

    if(filter_method!=PNG_FILTER_TYPE_DEFAULT) answer |= BAD_FILTER_TYPE;

    if(in(color_type, std::vector<int> {PNG_COLOR_TYPE_RGB,PNG_COLOR_TYPE_RGB_ALPHA,PNG_COLOR_TYPE_GRAY,PNG_COLOR_TYPE_GRAY_ALPHA,PNG_COLOR_TYPE_PALETTE}
)) {
    }

    int ps=getPixelSize();
    if(ps==0) answer |= BAD_BIT_DEPTH; // this will incidentally rule out forbidden bit depths
    else {
      if(strict_data_size) {
        if(data.size() != (size_t)ps*(size_t)width*(size_t)height) answer |= BAD_DATA_SIZE;
      }
      else {
        if(data.size() < (size_t)ps*(size_t)width*(size_t)height) answer |= BAD_DATA_SIZE;
      }
    }

    if(color_type==PNG_COLOR_TYPE_PALETTE) {
      if(palette.size()==0 || palette.size()>(1u << bit_depth)) answer |= BAD_PALETTE_SIZE; // for performance reasons, we --do not-- check if all bytes in data are lesss than palette.size(), though if not, the image is invalid
    }

    return answer;
  }

  // **********

  // Convenience functions for inserting text chunks
  // In the current implementation we perform a copy of the strings
  // given in parameters.
  // Would it be useful to [provide also/replace with] a version
  // with a move instead of a copy?

  // **********

  void addLatin1Text(const std::string &key, const std::string &text) {
    pngText t;
    t.type = PNG_TEXT_COMPRESSION_NONE;
    t.key = key; // copy: this could be optimized? 
    t.text = text;
    text_list.push_back(t);
  }

  void addLatin1TextZ(const std::string &key, const std::string &text) {
    pngText t;
    t.type = PNG_TEXT_COMPRESSION_zTXt;
    t.key = key;
    t.text = text;
    text_list.push_back(t);
  }

  void addUTF8Text(const std::string &key, const std::string &text, const std::string &lang, const std::string &lang_key) {
    pngText t;
    t.type = PNG_ITXT_COMPRESSION_NONE;
    t.key = key;
    t.text = text;
    t.lang = lang;
    t.lang_key = lang_key;
    text_list.push_back(t);
  }

  void addUTF8TextZ(const std::string &key, const std::string &text, const std::string &lang, const std::string &lang_key) {
    pngText t;
    t.type = PNG_ITXT_COMPRESSION_zTXt;
    t.key = key;
    t.text = text;
    t.lang = lang;
    t.lang_key = lang_key;
    text_list.push_back(t);
  }
 
  // **********

  // Load and save functions

  // **********

  int load(std::string fileName) {
    // Open file
    FILE* file=fopen(fileName.c_str(),"rb");
    if(!file) return(FILE_OPEN_FAILED);
    int code=load(file);
    fclose(file);
    return(code);
  }

  int load(FILE* file) {
    has_sRGB=true;
    has_gamma=false;
    has_tRNS=false;
    has_background=true;

    // initialize pointers load_ptr, info_ptr, end_ptr related to
    // libpng's file read procedures
    png_structp load_ptr;
    png_infop info_ptr;
    png_infop end_info;

    int pixel_size,channel_size,channels;
    bool test;

    png_colorp temp_palette;
    png_text* text_ptr2;
    int num_text;
    int num_palette;
    int srgb_intent;
    double gam;
    png_color_16p bkgd; // pointer!! (no need to free it)

    png_byte *trans_alpha; // libpng does the free
    int num_trans;
    png_color_16p trans_color=nullptr; // they use a pointer as a trick to 
                                       // allow for NULL to mean "undefined"

    load_ptr = png_create_read_struct
      (PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);
    if (!load_ptr) return INIT_FAILED;
  
    info_ptr = png_create_info_struct(load_ptr);
    if (!info_ptr) {
      png_destroy_read_struct(&load_ptr, (png_infopp)NULL, (png_infopp)NULL);
      return INIT_FAILED;
    }

    end_info = png_create_info_struct(load_ptr);
    if (!end_info) {
      png_destroy_read_struct(&load_ptr, &info_ptr, (png_infopp)NULL);
      return INIT_FAILED;
    }

    std::vector<png_byte*> rows(height);

    // jump point to handle libpng errors in the sequel
    if(setjmp(png_jmpbuf(load_ptr))) {
      png_destroy_read_struct(&load_ptr, &info_ptr, (png_infopp)NULL);
      return(LIBPNG_ERROR);
    };

    // WARNING: Since setjpm was done, there should be --no-- variable 
    // creation or allocation in this function after this point, 
    // otherwise longjpm will create a memory leak.

    // Init some of libpng's pointers
    png_init_io(load_ptr, file);
    png_read_info(load_ptr, info_ptr);

    // Read header
    test = png_get_IHDR(load_ptr, info_ptr,
                   &width, &height, &bit_depth, &color_type,
                   &interlace_type, &compression_type, &filter_method);
    if(!test) return(GET_HEADER_FAILED);

    // require at most one pixel channel per byte
    if(bit_depth<8) png_set_packing(load_ptr);

    channels=png_get_channels(load_ptr, info_ptr);
    channel_size=bit_depth<=8 ? 1 : 2;
    pixel_size=channels*channel_size;


    // Prepare data:
    // -allocate memory
    data.resize(height*width*pixel_size);
    // -create a dynamic array of pointers to the rows of allocated memory
    for(unsigned int j=0; j<height; j++) {
      rows[j]=((png_byte*) data.data())+(width*j)*pixel_size;
    }

    // Possibly ask for a byte swap in case of unusual endianness
    if(bit_depth==16 && !big_endian) png_set_swap(load_ptr);

    // set_swap is a transformation, we need to update the info according to the manual of libpng
    png_read_update_info(load_ptr,info_ptr);

    // Load the data
    png_read_image(load_ptr, rows.data());

//    if(bit_depth<8) bit_depth=8; // What??

    if(color_type==PNG_COLOR_TYPE_PALETTE) {
      if(!png_get_PLTE(load_ptr, info_ptr, &temp_palette, &num_palette)) return PALETTE_MISSING;

      palette.resize(num_palette);
      for(int i=0; i<num_palette; i++) {
        palette[i]=temp_palette[i];
      }

      if(png_get_tRNS(load_ptr, info_ptr, &trans_alpha, &num_trans, NULL)) {
        if(trans_alpha != nullptr) {
          transparency_palette.resize(num_trans);
          for(int i=0; i<num_trans; i++) {
            transparency_palette[i] = trans_alpha[i];
          }
        }  // if not, then there is a problem but we don't report it
        has_tRNS = true;
      }
    }
    else if(color_type==PNG_COLOR_TYPE_RGB) {
      if(png_get_tRNS(load_ptr, info_ptr, NULL, &num_trans, &trans_color)) {
        has_tRNS = true;
        transparent_color = *trans_color;
      }
    }
    else if(color_type==PNG_COLOR_TYPE_GRAY) {
      if(png_get_tRNS(load_ptr, info_ptr, NULL, &num_trans, &trans_color)) {
        has_tRNS = true;
        transparent_color = *trans_color;
      }
    } // note : we don't read tRNS data for the color types where it 
      // is not supposed to appear


    if(png_get_sRGB(load_ptr, info_ptr, &srgb_intent)) { has_sRGB=true; sRGB_intent=srgb_intent;}
    if(png_get_gAMA(load_ptr, info_ptr, &gam)) { decoding_gamma=1/gam; }

    if(png_get_bKGD(load_ptr, info_ptr, &bkgd)) {has_background = true; background_color=*bkgd; }
    
    png_read_end(load_ptr, end_info);
    
    // Retrieve comments
    png_get_text(load_ptr, info_ptr, &text_ptr2, &num_text);
    // when calling destroy, the data pointed to by text_ptr2 will be removed 

    // we deep-copy text_ptr2 into text
    text_list.resize(num_text);
    if(num_text>0) {
      for(int i=0; i<num_text; i++) {
        png_text *p2=&text_ptr2[i];
        pngText &t = text_list[i];
        if(p2->key)
          t.key = std::string(p2->key);
        else t.key.resize(0);
        if(p2->text)
          t.text = std::string(p2->text);
        else t.text.resize(0);
        t.type = p2->compression;
        if(p2->lang)
          t.lang = std::string(p2->lang);
        else t.lang.resize(0);
        if(p2->lang_key)
          t.lang_key = std::string(p2->lang_key);
        else t.lang_key.resize(0);
      }
    }
//    freeTextPtr(text_ptr2); // no need: png_destroy_read_struct will do it
     
    // Clean
    png_destroy_read_struct(&load_ptr, &info_ptr, &end_info);

    // Everything worked, return 0
    return(0);
  }

  int save(std::string fileName) {
    // Open file
    FILE* file=fopen(fileName.c_str(),"wb");
    if(!file) return(FILE_OPEN_FAILED);
    int code=save(file);
    fclose(file);
    return code;
  }

  int save(FILE* file) {

    png_structp save_ptr; 
    png_infop info_ptr;
    std::vector<png_text> text_2(text_list.size());

    int pixel_size,channel_size,channels;

    // image
    std::vector<png_bytep> rows;
                     // png_write_image() needs a C-style array
                     // of pointers to the lines of
                     // data in the picture, rows.data() provides such
                     // the std::vector class provides RAII

    save_ptr = png_create_write_struct(
      PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL );
    if (!save_ptr) return(INIT_FAILED); 

    info_ptr = png_create_info_struct(save_ptr);
    if (!info_ptr) {
      png_destroy_write_struct(&save_ptr,(png_infopp)NULL);
      return(INIT_FAILED);
    };

    if(text_list.size()>0) {
      for(size_t i=0; i<text_list.size(); i++) {
        // the next fields will not be destroyed by libpng
        // (see libpng's messy doc)
        text_2[i].compression = text_list[i].type;
        text_2[i].key  = const_cast<png_charp>(text_list[i].key.c_str());
        text_2[i].text = const_cast<png_charp>(text_list[i].text.c_str());
        if(text_list[i].type == PNG_TEXT_COMPRESSION_NONE
        || text_list[i].type == PNG_TEXT_COMPRESSION_zTXt) {
          text_2[i].text_length = text_list[i].text.size();
          text_2[i].itxt_length = 0;
          text_2[i].lang=nullptr;
          text_2[i].lang_key=nullptr;
        }
        else {
          text_2[i].text_length = 0;
          text_2[i].itxt_length = text_list[i].text.size();
          text_2[i].lang     = const_cast<png_charp>(text_list[i].lang.c_str());
          text_2[i].lang_key = const_cast<png_charp>(text_list[i].lang_key.c_str());
        }
      }
    }
  
    // jump point to handle libpng errors in the sequel
    if(setjmp(png_jmpbuf(save_ptr))) {
      png_destroy_write_struct(&save_ptr, &info_ptr);
      return(LIBPNG_ERROR);
    };

    // WARNING: Since setjpm was done, there should be --no-- variable 
    // creation or allocation in this function after this point, 
    // otherwise longjpm will create a memory leak.

    // initialisation
    png_init_io(save_ptr, file);

    // header
    png_set_IHDR(save_ptr, info_ptr, width, height,
                 bit_depth, color_type, interlace_type,
                 compression_type, filter_method);

    if(has_sRGB) {
      png_set_sRGB_gAMA_and_cHRM(save_ptr, info_ptr, sRGB_intent);
    } else {
      if(has_gamma)
        png_set_gAMA(save_ptr, info_ptr, 1.0/decoding_gamma);
    }

    if(has_background) {
      png_set_bKGD(save_ptr, info_ptr, &background_color);
    }

    // If there should be a palette
    if(color_type & PNG_COLOR_MASK_PALETTE)
      // then assing the palette
      png_set_PLTE(save_ptr, info_ptr, palette.data(), palette.size());
  
    // tRNS chunk
    if(has_tRNS) {
      if(color_type==PNG_COLOR_TYPE_PALETTE) {
        png_set_tRNS(save_ptr, info_ptr, transparency_palette.data(), transparency_palette.size(), NULL);
      }
      else if(color_type==PNG_COLOR_TYPE_RGB) {
        png_set_tRNS(save_ptr, info_ptr, nullptr, 1, &transparent_color);
      }
      else if(color_type==PNG_COLOR_TYPE_GRAY) {
        png_set_tRNS(save_ptr, info_ptr, nullptr, 1, &transparent_color);
      }
    } // note : we don't set tRNS chunk for the color types where it 
      // is not supposed to appear

    // If some text is to be saved
    if(text_list.size()>0) {
      // then assign it
      png_set_text(save_ptr, info_ptr, text_2.data(), text_list.size());
    }

    if(bit_depth<8) png_set_packing(save_ptr);

    if(bit_depth==16 && !big_endian) png_set_swap(save_ptr);

    // Now, serious stuff

    // write info header
    png_write_info(save_ptr, info_ptr);

    channels=png_get_channels(save_ptr, info_ptr);
    channel_size=bit_depth<=8 ? 1 : 2;
    pixel_size=channels*channel_size;

    rows.resize(height);
    for(unsigned int j=0; j<height; j++) { // so we subdivide char *image in lines
      rows[j]=((png_byte*) data.data())+(width*j)*pixel_size;
    };

    png_write_image(save_ptr, rows.data()); // This DOES NOT write 
                                            // the image in the file

    // The following DOES write the data on the file
    png_write_end(save_ptr, info_ptr);

    // Let us free the memory allocated by the various pnglib functions
    // we called ...
    png_destroy_write_struct(&save_ptr, &info_ptr);
  
    // Everything worked
    return(0);
  }

};
