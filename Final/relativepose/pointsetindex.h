// -*- c++ -*-

#ifndef POINTSETINDEX_H
#define POINTSETINDEX_H

#include <fstream>
#include <list>

#include "descriptor.h"

struct DescriptorTree {
  Descriptor descriptor;
  int children[2];
  int imagepointindex;
};


class PointSetIndex {
public:
  PointSetIndex(){}
  PointSetIndex(std::vector<Descriptor>& ps){
    build_tree(ps);
  }

  void build_tree(std::vector<Descriptor>& ps);

  std::vector<std::pair<int,int> > find_match(const Descriptor& d, int match_threshold);
  std::pair<int,int> best_match(const Descriptor& d, int match_threshold);

private:
  std::vector<DescriptorTree> index;
};


#endif
