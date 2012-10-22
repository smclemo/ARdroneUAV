/*
 * median.h
 *
 *  Created on: 22/02/2012
 *      Author: winston
 */

#ifndef MEDIAN_H_
#define MEDIAN_H_

#include <vector>
#include <TooN/TooN.h>

//Find the median using the Linfinity norm
TooN::Vector<2> findMedian(std::vector<TooN::Vector<2> > &in);

//Compare less than using L infinity norm
bool cmpl_Linf(TooN::Vector<2> const & lhs, TooN::Vector<2> const &rhs);

#endif /* MEDIAN_H_ */
