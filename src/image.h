#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <cassert>
#include <string>
#include <iostream>

// ====================================================================
// 24 bit color pixel
class Color {
public:
  Color(unsigned char red=255, unsigned char green=255, unsigned char blue=255) : r(red),g(green),b(blue) {}
  bool isWhite() const { return r==255 && g==255 && b==255; }
  unsigned char r,g,b;
};

// ====================================================================
// ====================================================================
// TEMPLATED IMAGE CLASS
//    can be saved and loaded from standard file formats:
//      .ppm    (when T == Color)
//      .pgm    (when T == int)

template <class T>
class Image {
public:
  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Image() : width(0), height(0), data(NULL) {}
  ~Image() { delete [] data; }

  // initialize an image of a specific size
  void Allocate(int w, int h) {
    width = w;
    height = h;
    delete [] data;
    if (width == 0 && height == 0) {
      data = NULL;
    } else {
      assert (width > 0 && height > 0);
      data = new T[width*height]; 
    }
  }

  // =========
  // ACCESSORS
  int Width() const { return width; }
  int Height() const { return height; }
  const T& GetPixel(int x, int y) const {
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);
    return data[y*width + x]; }

  // =========
  // MODIFIER
  void SetPixel(int x, int y, const T &value) {
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);
    data[y*width + x] = value; }

  // ===========
  // LOAD & SAVE
  bool Load(const std::string &filename);
  bool Save(const std::string &filename) const; 
  
private:
  // to prevent accidental usage...
  Image(const Image &image) { assert(0); }
  const Image& operator=(const Image &image) { assert(0); }

  // ==============
  // REPRESENTATION
  int width;
  int height;
  T *data;
};

#endif
