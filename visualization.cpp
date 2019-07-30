#include "visualization.h"


// Point and Bounding Box functions

std::ostream& operator<< (std::ostream &ostr, const Point &p) {
  ostr << "(" << p.x << "," << p.y << ")";
  return ostr;
}

std::ostream& operator<< (std::ostream& ostr, const BoundingBox& bbox) {
  ostr << bbox.getMin() << ":" << bbox.getMax();
  return ostr;
}

void BoundingBox::expand(const Point &a) {
  min.x = std::min(min.x,a.x);
  min.y = std::min(min.y,a.y);
  max.x = std::max(max.x,a.x);
  max.y = std::max(max.y,a.y);
}



// ======================================================================
// Copy data from a greyscale .pgm image into the RGB color .ppm image
// for visualization.  Darken all pixels of the image by 50% so that
// the visualization of color highlighting and bounding box
// visualization will be more clear.
void InitializeOutputFile(Image<int>& image, Image<Color> &out) {
  out.Allocate(image.Width(),image.Height());
  int w = image.Width();
  int h = image.Height(); 
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      int val = image.GetPixel(i,j);
      out.SetPixel(i,j,Color(val*0.5,val*0.5,val*0.5));
    }
  }
}

// ======================================================================
// Highlight this seed by adding 50% more color to the pixels in this
// block.  Alternate visualization through 3 different colors.
void HighlightSeed(Image<Color>& out, int which_color, const Point& p, int seed) {
  for (int x = p.x; x < p.x+seed; x++) {
    for (int y = p.y; y < p.y+seed; y++) {
      Color c = out.GetPixel(x,y);
      if (which_color % 3 == 0) {
        if (c.r < 128) {
          c.r += 128;
          out.SetPixel(x,y,c);
        }
      } else if (which_color % 3 == 1) {
        if (c.g < 128) {
          c.g += 128;
          out.SetPixel(x,y,c);
        }
      } else if (which_color % 3 == 2) {
        if (c.b < 128) {
          c.b += 128;
          out.SetPixel(x,y,c);
        }
      }
    }
  }
}

// ======================================================================
// Visualize the specified region by drawing a thin line around the
// frame of the bounding region.  Alternate visualization through 3
// different colors.
void DrawBoundingBox(Image<Color> &image, const BoundingBox &bbox, int which_color) {
  Color c;
  if (which_color % 3 == 0) {
    c = Color(255,0,0);
  } else if (which_color % 3 == 1) {
    c = Color(0,255,0);
  } else if (which_color % 3 == 2) {
    c = Color(0,0,255);
  }
  // Check the bounding box coordinates
  const Point& min = bbox.getMin();
  const Point& max = bbox.getMax();
  assert (min.x < max.x);
  assert (min.y < max.y);
  assert (min.x >= 0);
  assert (min.y >= 0);
  assert (max.x < image.Width());
  assert (max.y < image.Height());
  // Draw the frame
  for (int x = min.x; x <= max.x; x++) {
    image.SetPixel(x,min.y,c);
    image.SetPixel(x,max.y,c);
  }
  for (int y = min.y; y <= max.y; y++) {
    image.SetPixel(min.x,y,c);
    image.SetPixel(max.x,y,c);
  }
}

