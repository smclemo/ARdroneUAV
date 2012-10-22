#include "pointset.h"

#include "Tom_fast9.h"
#include "Tom_fast_score.h"
#include <stdlib.h>
#include "neon.h"
#include "halfsize_neon.h"
#include "nonmax.h"

//#include <android/log.h>

void PointSet::build_from_image(CVD::Image<unsigned char>& im1, Image<TooN::Vector<2, float> >& xy_lookup, bool const use_rhips){
	database.clear();

	//add_layer(im1, xy_lookup, 1, use_rhips);

	Image<unsigned char> halfimage= halfsize(im1);
	add_layer(halfimage, xy_lookup, 2, use_rhips);
	
	Image<unsigned char> quartimage= halfsize(halfimage);
	add_layer(quartimage, xy_lookup, 4, use_rhips);
}

void PointSet::add_layer(CVD::Image<unsigned char>& im, Image<TooN::Vector<2, float> >& xy_lookup, double const scale, bool const use_rhips){
//	Container<ImageRef> corners;
//	Container<int> corner_scores;
	Container<ImageRef> max_corners(1024);
//
//	Tom_fast9_detect(im, barrier, 11, corners);
////	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "FAST CORNERS %d", corners.size());
//	if(corners.size()>0)
//	{
//          qsort(&(corners[0]), corners.size(), sizeof(ImageRef), ImgRefCompare);
//	}
////	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "SORTED FAST CORNERS %d", corners.size());
//	compute_scores(im, corners, barrier, corner_scores);
////	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "COMPUTED FAST CORNERS %d", corners.size());
//	max_corners.clear();
//	nonmax_suppression(corners, corner_scores, max_corners);
//	//__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "MAX FAST CORNERS %d, FASTSCORES %d", max_corners.size(), corner_scores.size());
//
//	for(int i =0; i < max_corners.size(); i++)
//	{
//		__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "%d %d", max_corners[i][0], max_corners[i][1]);
//	}

//	od.detector(im, fast_barrier, max_corners, nCornersPerLayer);
	od.detectDistributed(im, fast_barrier, max_corners, 2);
	int const num_max_corners = max_corners.size();

	if(use_rhips)
	{
		for(int i=0; i<num_max_corners; i++){
			ImagePoint ip;
			ip.rbuild_from_image(im, max_corners[i], xy_lookup, scale);
			database.add(ip);
//			__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "%d %d", max_corners[i].x, max_corners[i].y);
		}
	}
	else{
		for(int i=0; i<num_max_corners; i++){
			ImagePoint ip;
			ip.build_from_image(im, max_corners[i], xy_lookup,scale);
			database.add(ip);
		}
	}
}


ImagePoint& PointSet::operator[](int i){
  return database[i];
}


