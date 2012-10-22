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

#ifndef CVD_VISION_H_
#define CVD_VISION_H_

#include <vector>
#include <memory>
#include <algorithm>

#include "vision_exceptions.h"
#include "image.h"
#include "internal/pixel_operations.h"
#include "utility.h"

namespace CVD{
/** Subsamples an image to 2/3 of its size by averaging 3x3 blocks into 2x2 blocks.
@param in input image
@param out output image (must be <code>out.size() == in.size()/3*2 </code>)
@throw IncompatibleImageSizes if out does not have the correct dimensions.
@ingroup gVision
*/
template<class C> void twoThirdsSample(const SubImage<C>& in, SubImage<C>& out)
{
    typedef typename Pixel::traits<C>::wider_type sum_type;
	if( (in.size()/3*2) != out.size())
        throw Exceptions::Vision::IncompatibleImageSizes(__FUNCTION__);
	
	for(int yy=0, y=0; y < in.size().y-2; y+=3, yy+=2)
		for(int xx=0, x=0; x < in.size().x-2; x+=3, xx+=2)
		{
			// a b c
			// d e f
			// g h i

			sum_type b = in[y][x+1]*2;
			sum_type d = in[y+1][x]*2;
			sum_type f = in[y+1][x+2]*2;
			sum_type h = in[y+2][x+1]*2;
			sum_type e = in[y+1][x+1];

			out[yy][xx]     = static_cast<C>((in[  y][  x]*4+b+d+e)/9);
			out[yy][xx+1]   = static_cast<C>((in[  y][x+2]*4+b+f+e)/9);
			out[yy+1][xx]   = static_cast<C>((in[y+2][  x]*4+h+d+e)/9);
			out[yy+1][xx+1] = static_cast<C>((in[y+2][x+2]*4+h+f+e)/9);
		}
}

/**
@overload
*/
void twoThirdsSample(const SubImage<byte>& in, SubImage<byte>& out);

  #ifndef DOXYGEN_IGNORE_INTERNAL
  namespace Internal
  {
  	template<class C> class twoThirdsSampler{};
	template<class C>  struct ImagePromise<twoThirdsSampler<C> >
	{
		ImagePromise(const SubImage<C>& im)
		:i(im)
		{}

		const SubImage<C>& i;
		template<class D> void execute(Image<D>& j)
		{
			j.resize(i.size()/3*2);
			twoThirdsSample(i, j);
		}
	};
  };
  template<class C> Internal::ImagePromise<Internal::twoThirdsSampler<C> > twoThirdsSample(const SubImage<C>& c)
  {
    return Internal::ImagePromise<Internal::twoThirdsSampler<C> >(c);
  }
  #else
  	///Subsamples an image by averaging 3x3 blocks in to 2x2 ones.
	/// Note that this is performed using lazy evaluation, so subsampling
	/// happens on assignment, and memory allocation is not performed if
	/// unnecessary.
    /// @param from The image to convert from
	/// @return The converted image
    /// @ingroup gVision
  	template<class C> Image<C> twoThirdsSample(const SubImage<C>& from);

  #endif

/// subsamples an image to half its size by averaging 2x2 pixel blocks
/// @param in input image
/// @param out output image, must have the right dimensions versus input image
/// @throw IncompatibleImageSizes if out does not have half the dimensions of in
/// @ingroup gVision
template <class T>
void halfSample(const BasicImage<T>& in, BasicImage<T>& out)
{
    typedef typename Pixel::traits<T>::wider_type sum_type;
    if( (in.size()/2) != out.size())
        throw Exceptions::Vision::IncompatibleImageSizes("halfSample");
    const T* top = in.data();
    const T* bottom = top + in.size().x;
    const T* end = top + in.totalsize();
    int ow = out.size().x;
    int skip = in.size().x + (in.size().x % 2);
    T* p = out.data();
    while (bottom < end) {      
      for (int j=0; j<ow; j++) {
	*p = static_cast<T>((sum_type(top[0]) + top[1] + bottom[0] + bottom[1])/4);
	p++;
	top += 2;
	bottom += 2;
      }
      top += skip;
      bottom += skip;
    }
}

void halfSample(const BasicImage<byte>& in, BasicImage<byte>& out);

/// subsamples an image to half its size by averaging 2x2 pixel blocks
/// @param in input image
/// @return The output image
/// @throw IncompatibleImageSizes if out does not have half the dimensions of in
/// @ingroup gVision
template <class T>
inline Image<T> halfSample(const BasicImage<T>& in)
{
	Image<T> out(in.size()/2);
	halfSample(in, out);
	return out;
}

/// subsamples an image repeatedly by half its size by averaging 2x2 pixel blocks.
/// This version will not create a copy for 0 octaves because it receives already
/// an Image and will reuse the data.
/// @param in input image
/// @param octaves number of halfsamplings 
/// @return The output image
/// @throw IncompatibleImageSizes if out does not have half the dimensions of in
/// @ingroup gVision
template <class T>
inline Image<T> halfSample( Image<T> in, unsigned int octaves){
    for( ;octaves > 0; --octaves){
        in = halfSample(in);
    }
    return in;
}

/// thresholds an image by setting all pixel values below a minimum to 0 and all values above to a given maximum
/// @param im input image changed in place
/// @param minimum threshold value
/// @param hi maximum value for values above the threshold
/// @ingroup gVision
template <class T>
void threshold(BasicImage<T>& im, const T& minimum, const T& hi)
{
  typename BasicImage<T>::iterator it = im.begin();
  typename BasicImage<T>::iterator end = im.end();
  while (it != end) {
    if (*it < minimum)
      *it = T();
    else
      *it = hi;
    ++it;
  }
}

/// computes mean and stddev of intensities in an image. These are computed for each component of the
/// pixel type, therefore the output are two pixels with mean and stddev for each component.
/// @param im input image
/// @param mean pixel element containing the mean of intensities in the image for each component
/// @param stddev pixel element containing the standard deviation for each component
/// @ingroup gVision
template <class T>
void stats(const BasicImage<T>& im, T& mean, T& stddev)
{
    const int c = Pixel::Component<T>::count;
    double v;
    double sum[c] = {0};
    double sumSq[c] = {0};
    const T* p = im.data();
    const T* end = im.data()+im.totalsize();
    while (p != end) {
        for (int k=0; k<c; k++) {
            v = Pixel::Component<T>::get(*p, k);
            sum[k] += v;
            sumSq[k] += v*v;
        }
        ++p;
    }
    for (int k=0; k<c; k++) {
        double m = sum[k]/im.totalsize();
        Pixel::Component<T>::get(mean,k) = (typename Pixel::Component<T>::type)m;
        sumSq[k] /= im.totalsize();
        Pixel::Component<T>::get(stddev,k) = (typename Pixel::Component<T>::type)sqrt(sumSq[k] - m*m);
    }
}

/// a functor multiplying pixels with constant value.
/// @ingroup gVision
template <class T>
struct multiplyBy
{
  const T& factor;
  multiplyBy(const T& f) : factor(f) {};
  template <class S> inline S operator()(const S& s) const {
    return s * factor;
  }
};

template <class S, class T, int Sn=Pixel::Component<S>::count, int Tn=Pixel::Component<T>::count> struct Gradient;
template <class S, class T> struct Gradient<S,T,1,2> {
  typedef typename Pixel::Component<S>::type SComp;
  typedef typename Pixel::Component<T>::type TComp;
  typedef typename Pixel::traits<SComp>::wider_type diff_type;
  static void gradient(const BasicImage<S>& I, BasicImage<T>& grad) {
    int w = I.size().x;
    typename BasicImage<S>::const_iterator s = I.begin() + w + 1;
    typename BasicImage<S>::const_iterator end = I.end() - w - 1;
    typename BasicImage<T>::iterator t = grad.begin() + w + 1;
    while (s != end) {
      Pixel::Component<T>::get(*t, 0) = Pixel::scalar_convert<TComp,SComp,diff_type>(diff_type(*(s+1)) - *(s-1));
      Pixel::Component<T>::get(*t, 1) = Pixel::scalar_convert<TComp,SComp,diff_type>(diff_type(*(s+w)) - *(s-w));
      s++;
      t++;
    }
    zeroBorders(grad);
  }
};

/// computes the gradient image from an image. The gradient image contains two components per pixel holding
/// the x and y components of the gradient.
/// @param im input image
/// @param out output image, must have the same dimensions as input image
/// @throw IncompatibleImageSizes if out does not have same dimensions as im
/// @ingroup gVision
template <class S, class T> void gradient(const BasicImage<S>& im, BasicImage<T>& out)
{
  if( im.size() != out.size())
    throw Exceptions::Vision::IncompatibleImageSizes("gradient");
  Gradient<S,T>::gradient(im,out);
}


#ifndef DOXYGEN_IGNORE_INTERNAL
void gradient(const BasicImage<byte>& im, BasicImage<short[2]>& out);
#endif


template <class T, class S, typename Precision> inline void sample(const SubImage<S>& im, Precision x, Precision y, T& result)
{
  typedef typename Pixel::Component<S>::type SComp;
  typedef typename Pixel::Component<T>::type TComp;
  const int lx = (int)x;
  const int ly = (int)y;
  x -= lx;
  y -= ly;
  for(unsigned int i = 0; i < Pixel::Component<T>::count; i++){
    Pixel::Component<T>::get(result,i) = Pixel::scalar_convert<TComp,SComp>(
        (1-y)*((1-x)*Pixel::Component<S>::get(im[ly][lx],i) + x*Pixel::Component<S>::get(im[ly][lx+1], i)) +
          y * ((1-x)*Pixel::Component<S>::get(im[ly+1][lx],i) + x*Pixel::Component<S>::get(im[ly+1][lx+1],i)));
  }
 }

template <class T, class S, typename Precision> inline T sample(const SubImage<S>& im, Precision x, Precision y){
    T result;
    sample( im, x, y, result);
    return result;
}

inline void sample(const SubImage<float>& im, double x, double y, float& result)
{
    const int lx = (int)x;
    const int ly = (int)y;
    const int w = im.row_stride();
    const float* base = im[ly]+lx;
    const float a = base[0];
    const float b = base[1];
    const float c = base[w];
    const float d = base[w+1];
    const float e = a-b;
    x-=lx;
    y-=ly;
    result = (float)(x*(y*(e-c+d)-e)+y*(c-a)+a);
}


/// flips an image vertically in place.
template <class T> void flipVertical( Image<T> & in )
{
  int w = in.size().x;
  std::vector<T> buf(w);
  T* buffer = &buf[0];
  T * top = in.data();
  T * bottom = top + (in.size().y - 1)*w;
  while( top < bottom )
  {
    std::copy(top, top+w, buffer);
    std::copy(bottom, bottom+w, top);
    std::copy(buffer, buffer+w, bottom);
    top += w;
    bottom -= w;
  }
}

/// flips an image horizontally in place.
template <class T> void flipHorizontal( Image<T> & in )
{
  int w = in.size().x;
  int h = in.size().y;
  std::auto_ptr<T> buffer_auto(new T[w]);
  T* buffer = buffer_auto.get();
  T * left = in.data();
  T * right = left + w;
  int row = 0;
  while(row < h)
  {
    std::copy(left, right, buffer);
    std::reverse_copy(buffer, buffer+w-1, left);
    row++;
    left += w;
    right += w;
  }
}


namespace median {
    template <class T> inline T median3(T a, T b, T c) {
	if (b<a)
	    return std::max(b,std::min(a,c));
	else
	    return std::max(a,std::min(b,c));	
    }
    
    template <class T> inline void sort3(T& a, T& b, T& c) {
	using std::swap;
	if (b<a) swap(a,b);
	if (c<b) swap(b,c);
	if (b<a) swap(a,b);
    }
    
    template <class T> T median_3x3(const T* p, const int w) {
	T a = p[-w-1], b = p[-w], c = p[-w+1], d=p[-1], e=p[0], f=p[1], g=p[w-1], h=p[w], i=p[w+1];
	sort3(a,b,c);
	sort3(d,e,f);
	sort3(g,h,i);
	e = median3(b,e,h);
	g = std::max(std::max(a,d),g);
	c = std::min(c,std::min(f,i));
	return median3(c,e,g);
    }
    
    template <class T> void median_filter_3x3(const T* p, const int w, const int n, T* out)
    {
	T a = p[-w-1], b = p[-w], d=p[-1], e=p[0], g=p[w-1], h=p[w];
	sort3(a,d,g);
	sort3(b,e,h);
	for (int j=0; j<n; ++j, ++p, ++out) {
	    T c = p[-w+1], f = p[1], i = p[w+1];
	    sort3(c,f,i);
	    *out = median3(std::min(std::min(g,h),i), 
			   median3(d,e,f), 
			   std::max(std::max(a,b),c));
	    a=b; b=c; d=e; e=f; g=h; h=i;
	}
    }
}

    template <class T> void median_filter_3x3(const SubImage<T>& I, SubImage<T> out)
    {
	assert(out.size() == I.size());
	const int s = I.row_stride();
	const int n = I.size().x - 2;
	for (int i=1; i+1<I.size().y; ++i)
	    median::median_filter_3x3(I[i]+1, s, n, out[i]+1);
    }

void median_filter_3x3(const SubImage<byte>& I, SubImage<byte> out);

//template<class T>


}; // namespace CVD

#endif // CVD_VISION_H_
