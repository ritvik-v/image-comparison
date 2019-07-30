/**
 * Image seed block class
**/

#ifndef _SEED_H_
#define _SEED_H_

#include "image.h"
#include "visualization.h"

class Seed {
    public:
    // Constructor
    Seed(int w, int h, std::vector<int> s, const Image<int>* i) : p(w, h), seed(s), image(i) {}
    // Accessor
    const std::vector<int>& getSeedValues() const { return seed; }
    // Representation
    Point p;
    std::vector<int> seed;
    const Image<int>* image;
};

bool operator==(const Seed& a, const Seed& b) {
        if (a.image == b.image && a.seed == b.seed && a.p.x == b.p.x && a.p.y == b.p.y) return true;
        return false;
    }

class SeedHashFunctor {
    public:
    std::size_t operator() (const Seed& s) const {
        std::vector<int> values = s.getSeedValues();
        unsigned int hash = 1315423911;
        for (int i = 0; i < values.size(); ++i) {
            hash ^= ((hash << 5) + values[i] + (hash >> 2));
        }
        
        return (std::size_t)hash;
    }
};

#endif