/*
	This file is part of the CVD Library.

	Copyright (C) 2005 The Authors

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
//-*- c++ -*-
//////////////////////////////////////////////////////////////////////////
//                                                                      //
//  CVD::image.h                                                        //
//                                                                      //
//  Definitions for of template class CVD::Image, fast_image		//
//                                                                      //
//  derived from IPRS_* developed by Tom Drummond                       //
//	Reworked to provide class heirachy and memory managementby E. Rosten//
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef CVD_IMAGE_H
#define CVD_IMAGE_H

#include "image_ref.h"
#include <string.h>

namespace CVD {

template<typename T>
class Image {
public:
	inline Image() {
		my_data=0;
		my_refcount=0;
		my_size=ImageRef_zero;
		my_stride=0;
	}
	inline Image(ImageRef sz) {
		my_data = new T[sz.x*sz.y];
		my_refcount = new int(1);
		my_size=sz;
		my_stride = sz.x;
	}

	inline Image(T* d, ImageRef sz, int st) {
		my_data=d;
		my_refcount= 0;
		my_size=sz;
		my_stride=st;
	}

	inline Image(const Image<T>& from) {
		my_data = from.my_data;
		my_refcount = from.my_refcount;
		my_size = from.my_size;
		my_stride = from.my_stride;
		if(my_refcount){
			(*my_refcount)++;
		}
	}
	
	inline ~Image(){
		delete_current();
	}

	inline void delete_current(){
		if(my_refcount) {
			(*my_refcount)--;
			if(*my_refcount ==0) {
				delete[] my_data;
				delete my_refcount;
			}
		}
	}

	T* data() {
		return my_data;
	}
	const T* data() const {
		return my_data;
	}

	ImageRef size() const {
		return my_size;
	}

	int stride() const {
		return my_stride;
	}

	bool in_image(const ImageRef& pos) const {
		return(pos.x >= 0 && pos.x < my_size.x && pos.y >= 0 && pos.y < my_size.y);
	}

	inline T& operator[] (const ImageRef& pos) {
		return my_data[pos.y*my_stride+pos.x];
	}
	inline const T& operator[] (const ImageRef& pos) const {
		return my_data[pos.y*my_stride+pos.x];
	}

        inline T* operator[](int const row)
        {
            return my_data+row*my_stride;
        }

        inline const T* operator[](int const row) const
        {
            return my_data+row*my_stride;
        }

	inline void operator= (const Image<T>& im){
		delete_current();
		my_data = im.my_data;
		my_refcount = im.my_refcount;
		if(my_refcount){
			(*my_refcount)++;
		}
		my_stride = im.my_stride;
	}

	inline void make_unique(){
		if(my_refcount){
			if((*my_refcount)==1){
				return;
			} else {
				(*my_refcount)--;
			}
		}

		T* d = new T[my_size.x*my_size.y];
		memcpy(d, my_data, my_size.x * my_size.y * sizeof(T));
		my_data = d;
		my_refcount = new int(1);
	}

	// Here be dragons!
	// If a sub_image is made of a refcounted image whose refcount drops to zero,
	// then the sub_image becomes invalid
	inline Image<T> sub_image(const ImageRef& start, const ImageRef& size) {
		return Image<T>(&((*this)[start]), size, my_stride);
	}

	void fill(const T& val){
		ImageRef scan;
		for(scan.y=0; scan.y < my_size.y; scan.y++){
			for(scan.x=0; scan.x < my_size.x; scan.x++){
				(*this)[scan] = val;
			}
		}
	}




private:
	T* my_data;
	int* my_refcount; // if refcount is zero we don't own our data.  NB refcount is not threadsafe!!!
	ImageRef my_size;
	int my_stride;
};



};

#endif
