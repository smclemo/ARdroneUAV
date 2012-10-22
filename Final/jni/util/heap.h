#ifndef HEAP_H
#define HEAP_H

template<class T, template<typename> class Cont>
void make_min_heap (Cont<T>& c) {
	const int n = c.size();
    for (int v=n/2-1; v>=0; v--) {
        min_downheap (c,v);
    }
}

template<class T, template<typename> class Cont>
void min_downheap(Cont<T>& c, int v) {
	const int n = c.size();
	int w=2*v+1;    // first descendant of v
	while (w<n) {
		if (w+1<n) {    // is there a second descendant?
			if (c[w+1]<c[w]) {
				w++;
			}
		}
		// w is the descendant of v with minimum label

		if (!(c[w] < c[v])) return;  // v has heap property
		// otherwise
		T temp = c[v];
		c[v]=c[w];
		c[w]=temp;

		v=w;        // continue
		w=2*v+1;
	}
}


template<class T, template<typename> class Cont>
T pop_min_heap(Cont<T>& c){
	T result = c[0];
	int last = c.size()-1;
	c[0] = c[last];
	c.set_size(last);
	min_downheap(c,0);
	return result;
}


#endif
