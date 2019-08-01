#include <vector>
#include <unordered_set>
#include <iomanip>
#include <climits>

#include "image.h"
#include "seed.h"
#include "visualization.h"



// ======================================================================
// Helper function to read the optional arguments and filenames from
// the command line.
void parse_arguments(int argc, char* argv[], 
                     std::string& method, int& seed, int& table_size, float& compare,
                     std::vector<std::string>& filenames) {
  for (int i = 1; i < argc; i++) {
    if (argv[i] == std::string("-method")) {
      i++;
      assert (i < argc);
      method = argv[i];
      assert (method == "simple" || method == "hashtable");
    } else if (argv[i] == std::string("-seed")) {
      i++;
      assert (i < argc);
      seed = atoi(argv[i]);
      assert (seed >= 1);
    } else if (argv[i] == std::string("-table")) {
      i++;
      assert (i < argc);
      table_size = atoi(argv[i]);
      assert (table_size >= 1);
    } else if (argv[i] == std::string("-compare")) {
      i++;
      assert (i < argc);
      compare = atof(argv[i]);
      assert (compare > 0.0 && compare <= 1.0);
    } else {
      filenames.push_back(argv[i]);
    }
  }
  assert (filenames.size() > 0);
}


// ======================================================================
// This simple algorithm is rather inefficient, and may not find the
// largest overlapping subregion.  But it will find a subregion match
// of size seed x seed, if one exists.
void SimpleCompare(const Image<int>& a, const Image<int>& b, 
                   Image<Color> &out, int which_color,int seed, float& percent,
                   std::vector<std::pair<BoundingBox,BoundingBox> >& regions) {

  // First, find a small seed that matches
  bool found = false;
  Point offset_a;
  Point offset_b;

  // Search over all possible points in image a
  for (int i = 0; i <= a.Width()-seed && !found; i++) {
    for (int j = 0; j <= a.Height()-seed && !found; j++) {
      // Search over all possible points in image b
      for (int i2 = 0; i2 <= b.Width()-seed && !found; i2++) {
        for (int j2 = 0; j2 <= b.Height()-seed && !found; j2++) {
          bool match = true;
          // Check for small seed match
          for (int x = 0; x < seed && match; x++) {
            for (int y = 0; y < seed && match; y++) {
              if (a.GetPixel(i+x,j+y) != b.GetPixel(i2+x,j2+y)) {
                match = false;
              }
            }
          }
          if (match) {
            // break out of these loops!
            HighlightSeed(out,which_color,Point(i,j),seed);
            found = true;
            offset_a = Point(i,j);
            offset_b = Point(i2,j2);
          }
        }
      }
    }
  }
  if (!found) {
    // no match between these images
    percent = 0.0;
    return;
  } 

  int width = seed;
  int height = seed;

  //
  //
  // ASSIGNMENT:  COMPLETE THIS IMPLEMENTATION
  //
  //

  // First, expand the region match widthwise, until we hit the right
  // edge of one of the images or a mismatched pixel.

  while (width < a.Width() - offset_a.x - 1 && width < b.Width() - offset_b.x - 1) {
    if (a.GetPixel(offset_a.x + width, offset_a.y) != b.GetPixel(offset_b.x + width, offset_b.y)) break;
    width++;
  }

  // Then, expand the region match heightwise, until we hit the top
  // edge of one of the images or a mismatched pixel.

  while (height < a.Height() - offset_a.y - 1 && height < b.Height() - offset_b.y - 1) {
    if (a.GetPixel(offset_a.x, offset_a.y + height) != b.GetPixel(offset_b.x, offset_b.y + height)) break;
    height++;
  }

  //
  //
  //

  BoundingBox bbox_a(offset_a,Point(offset_a.x+width,offset_a.y+height));
  BoundingBox bbox_b(offset_b,Point(offset_b.x+width,offset_b.y+height));
  regions.push_back(std::make_pair(bbox_a,bbox_b));
  // return fraction of pixels
  percent = bbox_a.Width()*bbox_a.Height() / float (a.Width()*a.Height());
}

// ======================================================================

void HashCompare(const Image<int>& a, const Image<int>& b, 
                   Image<Color> &out, int which_color, int seed, float& percent,
                   std::vector<std::pair<BoundingBox,BoundingBox> > & regions, float compare, int table) {
  // hash all seed blocks in A
  std::unordered_set<Seed,SeedHashFunctor> HashTable(table);
  std::vector<int> seedBlock;

  for (int i = 0; i < a.Width() - seed; i++) {
    for (int j = 0; j < a.Height() - seed; j++) {
      for (int k = 0; k < seed; k++) {
        for (int m = 0; m < seed; m++) seedBlock.push_back(a.GetPixel(i + k, j + m));
      }
      
      HashTable.insert(Seed(i, j, seedBlock, &a));
      seedBlock.clear();
    }
  }
  
  // hash all seed blocks in B
  for (int i = 0; i < b.Width() - seed; i++) {
    for (int j = 0; j < b.Height() - seed; j++) {
      for (int k = 0; k < seed; k++) {
        for (int m = 0; m < seed; m++) seedBlock.push_back(b.GetPixel(i + k, j + m));
      }
      
      HashTable.insert(Seed(i, j, seedBlock, &b));
      seedBlock.clear();
    }
  }

  // traverse compare percentage of hash table, and look for matches
  Point top_a(0, 0), bottom_a(INT_MAX, INT_MAX), top_b(0, 0), bottom_b(INT_MAX, INT_MAX);
  bool no_match_found = true; // checks if the images do not match at all
  const Image<int>* img;
  std::vector<int> values;
  Point p;
  for (unsigned int i = 0; i < HashTable.bucket_count() * compare; i++) {
    if (HashTable.bucket_size(i) > 1) { // if there are matching seeds present
      std::unordered_set<Seed,SeedHashFunctor>::local_iterator itr = HashTable.begin(i);
      // Store the image containing the first seed, and the values of the first seed, and the appropriate points
      img = (*itr).image;
      p = (*itr).p;
      values = (*itr).getSeedValues();
      bool found = false; // stores if seeds from two different images are found
      itr++;
      while (!found && itr != HashTable.end(i)) {
        if ((*itr).image != img) { // if the seeds are from two different images
          for (int j = 0; j < seed * seed; j++) { // ensure not a false positive
            if (values[j] != (*itr).seed[j]) {
              found = false;
              break;
            }
            if (j == seed * seed - 1) {
              found = true;
              no_match_found = false;
            }
          }

          if (found) { // an appropriate seed match has been found
            if (img == &a) { // Check if either seed will extend the boundary of the bounding box
            if (p.x + seed > top_a.x) top_a.x = p.x + seed;
            if (p.y + seed > top_a.y) top_a.y = p.y + seed;
            if (p.x < bottom_a.x) bottom_a.x = p.x;
            if (p.y < bottom_a.y) bottom_a.y = p.y;

            if ((*itr).p.x + seed > top_b.x) top_b.x = (*itr).p.x + seed;
            if ((*itr).p.y + seed > top_b.y) top_b.y = (*itr).p.y + seed;
            if ((*itr).p.x < bottom_b.x) bottom_b.x = (*itr).p.x;
            if ((*itr).p.y < bottom_b.y) bottom_b.y = (*itr).p.y;
            } else {
            if (p.x + seed > top_b.x) top_b.x = p.x + seed;
            if (p.y + seed > top_b.y) top_b.y = p.y + seed;
            if (p.x < bottom_b.x) bottom_b.x = p.x;
            if (p.y < bottom_b.y) bottom_b.y = p.y;

            if ((*itr).p.x + seed > top_a.x) top_a.x = (*itr).p.x + seed;
            if ((*itr).p.y + seed > top_a.y) top_a.y = (*itr).p.y + seed;
            if ((*itr).p.x < bottom_a.x) bottom_a.x = (*itr).p.x;
            if ((*itr).p.y < bottom_a.y) bottom_a.y = (*itr).p.y;
            }

            // Highlight the seeds
            if (img == &a) HighlightSeed(out, which_color, p, seed);
            else HighlightSeed(out, which_color, (*itr).p, seed);
          } 
        }
        itr++;
      }
    }
  }

  if (no_match_found) {
    percent = 0;
    return;
  }

  BoundingBox bbox_a(bottom_a,top_a);
  BoundingBox bbox_b(bottom_b,top_b);
  regions.push_back(std::make_pair(bbox_a,bbox_b));

  // return fraction of pixels
  percent = bbox_a.Width()*bbox_a.Height() / float (a.Width()*a.Height());
}

// ======================================================================

int main(int argc, char* argv[]) {

  // default command line argument values
  std::string method = "simple";
  int seed = 5;
  int table_size = 1000000;
  float compare = 0.05;
  std::vector<std::string> filenames;
  parse_arguments(argc,argv,method,seed,table_size,compare,filenames);


  // Load all of the images
  std::vector<Image<int> > images(filenames.size());
  for (int i = 0; i < filenames.size(); i++) {
    images[i].Load(filenames[i]);
  }

  // Loop over all input images, comparing to every other input image
  for (int a = 0; a < filenames.size(); a++) {
    std::cout << filenames[a] << std::endl;
    // prepare a color visualization image for each input file
    Image<Color> out;
    InitializeOutputFile(images[a],out);
    int which_color = -1;
    for (int b = 0; b < filenames.size(); b++) {
      if (a == b) continue;
      which_color++;
      
      // Find the one (or more!) subregions that match between this pair of images
      std::vector<std::pair<BoundingBox,BoundingBox> > regions;
      float percent = 0.0;
      if (method == "simple") {
        SimpleCompare(images[a],images[b],out,which_color,seed,percent,regions);
      } else {
        assert (method == "hashtable");

        //
        //
        // ASSIGNMENT:  IMPLEMENT THE HASHTABLE MATCHING ALGORITHM
        //
        //

        HashCompare(images[a],images[b],out,which_color,seed,percent,regions,compare,table_size);

      } 

      std::cout << std::right << std::setw(7) << std::fixed << std::setprecision(1) 
                << 100.0 * percent << "% match with " << std::left << std::setw(20) << filenames[b];

      for (int i = 0; i < regions.size(); i++) {
        std::cout << "   " << regions[i].first << " similar to " << regions[i].second;
        // add the frame data to the visualization image
        DrawBoundingBox(out,regions[i].first,which_color);
      }
      std::cout << std::endl;
      // Save the visualization image
      std::string f = "output_" + filenames[a].substr(0,filenames[a].size()-4) + ".ppm";
      out.Save(f);
    }
  }
}
