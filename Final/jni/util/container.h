#ifndef CONTAINER_H
#define CONTAINER_H

#include <stdlib.h>
#include <string.h>

#define DEBUG_TAG "Tom-Native-stuff"


template<typename T>
class Container {
public:
	Container(int mx_sz = 0) {
		my_max_size=mx_sz;
		my_size=0;
		my_data = new T[my_max_size];
	}

	~Container() {
		delete[] my_data;
	}

	void set_max_size(int mx_sz){
		//printf("changing size from %d to %d\n", my_max_size, mx_sz);

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
			set_max_size(sz);
		}
		my_size=sz;
	}

	int size() const {
		return my_size;
	}

	void add(const T& val) {
		if(my_max_size==0){
			set_max_size(1);
		} else if(my_size == my_max_size) {
			set_max_size(my_max_size*2);
		}
		my_data[my_size] = val;
		my_size++;
	}

	void clear(){
		my_size=0;
	}

	T& operator[](int i) {
		if(i<0 || i>= my_size){
			//printf("access at %d with size = %d and max size = %d", i, my_size, my_max_size);
		}
		return my_data[i];
	}

	const T& operator[](int i) const {
		if(i<0 || i>= my_size){
			//printf("access at %d with size = %d and max size = %d", i, my_size, my_max_size);
		}
		return my_data[i];
	}


private:
	int my_max_size;
	int my_size;
	T* my_data;
};


#endif
