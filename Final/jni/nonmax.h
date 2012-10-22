#ifndef NONMAX_H
#define NONMAX_H

#include "cvd_lite/image.h"
#include <utility>
#include <vector>
#define Compare(X, Y) ((X)>(Y))

#define DEBUG_TAG "Tom-Native-stuff"


template <template<typename> class C1, template<typename> class C2, template<typename>  class C3 >
void nonmax_suppression(const C1<CVD::ImageRef>& corners, const C2<int>& scores, C3<CVD::ImageRef>& max_corners) {
	if(corners.size()==0){
		return;
	}

	int last_row;
	int* row_start;
	int i, j;
	const int sz = corners.size();

	/*Point above points (roughly) to the pixel above the one of interest, if there
    is a feature there.*/
	int point_above = 0;
	int point_below = 0;

	/* Find where each row begins
	   (the corners are output in raster scan order). A beginning of -1 signifies
	   that there are no corners on that row. */

	last_row=corners[corners.size()-1].y;

	row_start = new int[last_row+1];

	for(i=0; i < last_row+1; i++) {
		row_start[i] = -1;
	}

	for(i=sz-1; i>=0; i--){
		row_start[corners[i].y] = i;
	}

	for(i=0; i < sz; i++) {

		int score = scores[i];
		CVD::ImageRef pos = corners[i];

		/*Check left */
		if(i > 0)
			if(corners[i-1].x == pos.x-1 && corners[i-1].y == pos.y && Compare(scores[i-1], score))
				continue;

		/*Check right*/
		if(i < (sz - 1))
			if(corners[i+1].x == pos.x+1 && corners[i+1].y == pos.y && Compare(scores[i+1], score))
				continue;


		/*Check above (if there is a valid row above)*/
		if(pos.y != 0 && row_start[pos.y - 1] != -1)
		{

			/*Make sure that current point_above is one
			  row above.*/
			if(corners[point_above].y != pos.y - 1) {
				point_above = row_start[pos.y-1];
			}


			/*Make point_above point to the first of the pixels above the current point,
			  if it exists.*/
			for(; corners[point_above].y < pos.y && corners[point_above].x < pos.x - 1; point_above++)
			{}

			for(j=point_above; corners[j].y < pos.y && corners[j].x <= pos.x + 1; j++)
			{
				int x = corners[j].x;
				if( (x == pos.x - 1 || x ==pos.x || x == pos.x+1) && Compare(scores[j], score))
					goto cont;
			}

		}

		/*Check below (if there is anything below)*/
		if(pos.y != last_row && row_start[pos.y + 1] != -1 && point_below < sz) /*Nothing below*/
		{
			if(corners[point_below].y != pos.y + 1)
				point_below = row_start[pos.y+1];

			/* Make point below point to one of the pixels belowthe current point, if it
			   exists.*/
			for(; point_below < sz && corners[point_below].y == pos.y+1 && corners[point_below].x < pos.x - 1; point_below++)
			{}

			for(j=point_below; j < sz && corners[j].y == pos.y+1 && corners[j].x <= pos.x + 1; j++)
			{
				int x = corners[j].x;
				if( (x == pos.x - 1 || x ==pos.x || x == pos.x+1) && Compare(scores[j],score))
					goto cont;
			}
		}

		max_corners.add(corners[i]);
		cont:
		;
	}

	delete[] row_start;
}

template <template<typename> class C1, template<typename> class C2>
void nonmax_suppression_with_scores(const C1<CVD::ImageRef>& corners, const C2<int>& scores, std::vector<std::pair<CVD::ImageRef, int> > &max_corners) {
        if(corners.size()==0){
                return;
        }

        int i, j;
        const int sz = corners.size();
        int const last_row = corners[sz-1].y; //Must be in rasterscan order

        /*Point above points (roughly) to the pixel above the one of interest, if there
    is a feature there.*/
        int point_above = 0;
        int point_below = 0;

        /* Find where each row begins
           (the corners are output in raster scan order). A beginning of -1 signifies
           that there are no corners on that row. */


        int row_start[last_row+1];

        for(i=0; i < last_row+1; i++) {
                row_start[i] = -1;
        }

        for(i=sz-1; i>=0; i--){
                row_start[corners[i].y] = i;
        }

        for(i=0; i < sz; i++) {
                int const score = scores[i];
                CVD::ImageRef const &pos = corners[i];
                /*Check left */
                if(i > 0)
                        if(corners[i-1].x == pos.x-1 && corners[i-1].y == pos.y && Compare(scores[i-1], score))
                                continue;

                /*Check right*/
                if(i < (sz - 1))
                        if(corners[i+1].x == pos.x+1 && corners[i+1].y == pos.y && Compare(scores[i+1], score))
                                continue;


                /*Check above (if there is a valid row above)*/
                if(pos.y != 0 && row_start[pos.y - 1] != -1)
                {

                        /*Make sure that current point_above is one
                          row above.*/
                        if(corners[point_above].y != pos.y - 1) {
                                point_above = row_start[pos.y-1];
//                                __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Row %d Point Above %d", pos.y-1, point_above);
                        }


                        /*Make point_above point to the first of the pixels above the current point,
                          if it exists.*/
                        for(; point_above < sz && corners[point_above].y < pos.y && corners[point_above].x < pos.x - 1; point_above++)
                        {}

                        for(j=point_above; corners[j].y < pos.y && corners[j].x <= pos.x + 1; j++)
                        {
                                int x = corners[j].x;
                                if( (x == pos.x - 1 || x ==pos.x || x == pos.x+1) && Compare(scores[j], score))
                                        goto cont;
                        }

                }

                /*Check below (if there is anything below)*/
                if(pos.y != last_row && row_start[pos.y + 1] != -1 && point_below < sz) /*Nothing below*/
                {
                        if(corners[point_below].y != pos.y + 1)
                                point_below = row_start[pos.y+1];

                        /* Make point below point to one of the pixels below the current point, if it
                           exists.*/
                        for(; point_below < sz && corners[point_below].y == pos.y+1 && corners[point_below].x < pos.x - 1; point_below++)
                        {}

                        for(j=point_below; j < sz && corners[j].y == pos.y+1 && corners[j].x <= pos.x + 1; j++)
                        {
                                int x = corners[j].x;
                                if( (x == pos.x - 1 || x ==pos.x || x == pos.x+1) && Compare(scores[j],score))
                                        goto cont;
                        }
                }
                max_corners.push_back(std::make_pair(pos, score));
                cont:
                ;
        }

}

#endif
