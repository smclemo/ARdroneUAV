#ifndef BOUNDED_CONTAINER_H
#define BOUNDED_CONTAINER_H

//#include <android/log.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_TAG "Tom-Native-stuff"


template<typename T>
class Bounded_Container {
public:
	Bounded_Container(int mx_sz = 0) {
		my_max_size=mx_sz;
		my_size=0;
		my_data = new T[my_max_size];
	}

	~Bounded_Container() {
		delete[] my_data;
	}

	void set_max_size(int mx_sz){
		T* nd = new T[mx_sz];
		int num_to_copy = mx_sz < my_size?mx_sz:my_size;
		memcpy(nd,my_data,num_to_copy*sizeof(T));
		delete[] my_data;
		my_data = nd;
		my_max_size=mx_sz;
		my_size=num_to_copy;
	}

	void set_size(int sz){
		if(sz > my_max_size){
			sz = my_max_size;
		}
		my_size=sz;
	}

	int size() const {
		if(my_size < my_max_size){
			return my_size;
		}
		return my_max_size;
	}

	void add(const T& val) {
		if(my_max_size==0) {return;}
		if(my_size<my_max_size) {
			my_data[my_size]=val;
			my_size++;
		} else {
			my_size++;
			long unsigned int trypos = mrand48(); // MUST be unsigned int before the % operation otherwise can get -ve result
			trypos%=my_size;
			if(trypos < my_max_size) {
				my_data[trypos] = val;
			}
		}
	}

	void clear(){
		my_size=0;
	}

	T& operator[](int i) {
		return my_data[i];
	}

	const T& operator[](int i) const {
		return my_data[i];
	}


private:
	int my_max_size;
	int my_size;
	T* my_data;
};

#endif
