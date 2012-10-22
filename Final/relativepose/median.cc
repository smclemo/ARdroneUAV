/*
 * median.cc
 *
 *  Created on: 22/02/2012
 *      Author: winston
 */
#include "median.h"
#include <algorithm>

TooN::Vector<2> findMedian(std::vector<TooN::Vector<2> > &in)
{
  size_t const n = in.size()/2;
  std::nth_element(in.begin(), in.begin() + n, in.end(), cmpl_Linf);
  return in[n];
}

bool cmpl_Linf(TooN::Vector<2> const & lhs, TooN::Vector<2> const &rhs)
{
  return std::max(lhs[0],lhs[1]) < std::max(rhs[0], rhs[1]);
}
