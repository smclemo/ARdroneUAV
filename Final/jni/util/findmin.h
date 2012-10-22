#ifndef FINDMIN_H
#define FINDMIN_H




template<class T, template<typename> class Cont>
int findmin (Cont<T>& c) {
	const int size = c.size();
	int best=size-1;
	for(int i=0; i<size-1; i++) {
		if(c[i] < c[best]){
			best = i;
		}
	}
	return best;
}


#endif
