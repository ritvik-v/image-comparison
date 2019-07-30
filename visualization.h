#ifndef __VISUALIZATION_H__
#define __VISUALIZATION_H__

#include <iostream>
#include "image.h"

// ======================================================================
// To store a 2D image coordinate

class Point {
public:
  Point(int i=0, int j=0) : x(i),y(j) {}
  int x;
  int y;
};

std::ostream& operator<< (std::ostream &ostr, const Point &p);


// ======================================================================
// To store the lower left and upper right coordinates of a region of
// an image. 

class BoundingBox {
public:
  BoundingBox(const Point& a = Point(0,0), const Point &b = Point(0,0)) : min(a),max(b) {}
  void expand(const Point &a);
  const Point& getMin() const { return min; }
  const Point& getMax() const { return max; }
  const int Width() const { return max.x - min.x + 1; }
  const int Height() const { return max.y - min.y + 1; }
private:
  Point min;
  Point max;
};

std::ostream& operator<< (std::ostream& ostr, const BoundingBox& bbox);


// ======================================================================

void InitializeOutputFile(Image<int>& image, Image<Color> &out);
void HighlightSeed(Image<Color>& out, int which_color, const Point& p, int seed);
void DrawBoundingBox(Image<Color> &image, const BoundingBox &bbox, int which_color);

#endif
