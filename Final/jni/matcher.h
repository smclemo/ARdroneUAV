#ifndef MATCHER_H
#define MATCHER_H

#include "descriptor.h"
#include "cvd_lite/image.h"
#include "util/container.h"
#include "util/heap.h"
#include "TooN/TooN.h"

#define DEBUG_TAG "Tom-Native-stuff"


struct Merger {
	int first;
	int second;
	int score;

	int operator < (const Merger& rhs) const {
		return (score < rhs.score);
	}
};

struct TreeNode {
	TreeNode(){
		init();
	}
	TreeNode(Descriptor d){
		desc=d;
		init();
	}
	TreeNode(Descriptor d, CVD::ImageRef p, float scale){
		desc=d;
		pos=p;
		init();
	}

	void init(){
		parent=-1;
		oldest_child=-1;
		older_sibling=-1;
		younger_sibling=-1;
		num_children=0;
	}


	Descriptor desc;
	int parent;
	int oldest_child;
	int older_sibling;
	int younger_sibling;
	int num_children;
	CVD::ImageRef pos;
	TooN::Vector<2> ref_pos;
};

//struct Tree {
//	Container<TreeNode> tree;
//
//	void merge(int c1, int c2){
//		int size = tree.size();
//		tree.set_size(size+1);
//		TreeNode& tn = tree[size];
//		tn.desc = tree[c1].desc | tree[c2].desc;
//		tn.parent=-1;
//		tn.oldest_child = c1;
//		tn.older_sibling=-1;
//		tn.younger_sibling=-1;
//
//		tree[c1].parent=size;
//		tree[c2].parent=size;
//		tree[c1].younger_sibling = c2;
//		tree[c2].older_sibling = c1;
//	}
//
//	void remove(int n){
//		TreeNode& tn = tree[n];
//		if(tn.older_sibling==-1) { // I am oldest sibling
//			tree[tn.parent].oldest_child = tn.oldest_child;
//		} else {
//			tree[tn.older_sibling].younger_sibling = tn.oldest_child;
//			tree[tn.oldest_child].older_sibling = tn.older_sibling;
//		}
//
//		// reparent all my children and make the last one point to my next sibling
//		int c=tn.oldest_child;
//		while(1){
//			tree[c].parent = tn.parent;
//			int c2 = tree[c].younger_sibling;
//			if(c2==-1){
//				tree[c].younger_sibling = tn.younger_sibling;
//				break;
//			}
//			c=c2;
//		}
//		tn.init();
//	}
//
//
//};



class Matcher{
public:
	Matcher(){
		my_tree.set_max_size(1024);
//		my_corners.set_max_size(1024);
//		my_descriptors.set_max_size(1024);
	}

	template <template<typename> class Cont1, template<typename> class Cont2>
	inline void build_from_corners(const Cont1<CVD::ImageRef>& corners, const Cont2<Descriptor>& descriptors, float scale){
		const int num_corners = corners.size();
		my_tree.set_max_size(2*num_corners);
		my_tree.set_size(num_corners);

		for(int i=0; i<num_corners; i++){
			my_tree[i].desc = descriptors[i];
			my_tree[i].pos = corners[i];
			my_tree[i].init();
		}

		//build initial search tree

		// first get a list of pairs of descriptors to merge and calculate the cost of each merge
		mergelist.set_max_size(num_corners*num_corners);
		mergelist.clear();
		for(int i=1; i<num_corners; i++){
			for(int j=0; j<i; j++){
				Merger merg;
				merg.first = i;
				merg.second = j;
				merg.score = (my_tree[i].desc | my_tree[j].desc).bitcount();
				mergelist.add(merg);
			}
		}

		//__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "made mergelist");


		// sort the list by increasing cost
		make_min_heap(mergelist);

		//__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "made heap");


		// repeatedly grab the merge of least cost
		// do it and update the sorted mergelist
		while(mergelist.size() > 0){
			Merger merg = pop_min_heap(mergelist);

			// check that neither node has a parent already
			if(my_tree[merg.first].parent!=-1 || my_tree[merg.second].parent != -1){
				continue;
			}

			// __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "merging %d and %d", merg.first, merg.second);

			merge(merg.first,merg.second);

			const int d = my_tree.size()-1;

			// now compute Mergers from the new descriptor to everything and add into the list;
			for(int i=0; i<d; i++){
				if(my_tree[i].parent==-1) {

					Merger merg;
					merg.first = d;
					merg.second = i;
					merg.score = (my_tree[i].desc | my_tree[d].desc).bitcount();
					mergelist.add(merg);
				}
			}
			make_min_heap(mergelist);
		}

		//__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "merged into tree");

		numpass.set_size(my_tree.size());
		for(int i=0; i<my_tree.size(); i++) {
			int count=0;
			for(int j=0; j<num_corners; j++){
				if(my_tree[i].desc.error_of(descriptors[j]) < the_max_error) {
					count++;
				}
			}
			numpass[i]=count;
		}

		//__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "counted passrate for each node");

		int pending=1;
		int num_removed=0;
		while(pending){
			pending=0;
			for(int i=my_tree.size()-2; i>=num_corners; i--){
				if(my_tree[i].num_children==0){
					continue;
				}
				int s = saving(i);
				if(s > 0) {
					// caculate the potential savings of all children
					int cs = 0;
					for(int c=my_tree[i].oldest_child; c!=-1; c=my_tree[c].younger_sibling){
						int sav = saving(c);
						if(sav > 0) {
							cs+= sav;
						}
					}
					if(s > cs){
						remove(i);
						num_removed++;
					} else {
						pending=1;
					}
				}
			}
		}
		//__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "deleted %d unwanted tree nodes", num_removed);
	}

	inline int saving(int n) const {

		const TreeNode& tn=my_tree[n];

		if(tn.num_children==0){
			return 0;
		}
		const int np= numpass[tn.parent];
		return np - (np - numpass[n])*tn.num_children;
	}


	void merge(int c1, int c2){
		int size = my_tree.size();
		my_tree.set_size(size+1);
		TreeNode& tn = my_tree[size];
		tn.desc = my_tree[c1].desc | my_tree[c2].desc;
		tn.parent=-1;
		tn.oldest_child = c1;
		tn.older_sibling=-1;
		tn.younger_sibling=-1;
		tn.num_children=2;

		my_tree[c1].parent=size;
		my_tree[c2].parent=size;
		my_tree[c1].younger_sibling = c2;
		my_tree[c2].older_sibling = c1;
	}

	void remove(int n){
		TreeNode& tn = my_tree[n];
		if(tn.older_sibling==-1) { // I am oldest sibling
			my_tree[tn.parent].oldest_child = tn.oldest_child;
		} else {
			my_tree[tn.older_sibling].younger_sibling = tn.oldest_child;
			my_tree[tn.oldest_child].older_sibling = tn.older_sibling;
		}

		// reparent all my children and make the last one point to my next sibling
		int c=tn.oldest_child;
		while(1){
			my_tree[c].parent = tn.parent;
			int c2 = my_tree[c].younger_sibling;
			if(c2==-1){
				my_tree[c].younger_sibling = tn.younger_sibling;
				if(tn.younger_sibling !=-1){
					my_tree[tn.younger_sibling].older_sibling = c;
				}
				break;
			}
			c=c2;
		}
		my_tree[tn.parent].num_children += tn.num_children-1;

		tn.init();
	}




	int match(const Descriptor& d){

		if(my_tree.size()==0) {
			return -1;
		}

		int best_dist = the_max_error;
		int best_match=-1;


		// alternative version using tree
		Container<int> possibles(100);
		possibles.add(my_tree.size()-1);
		while(possibles.size()>0){
			int last = possibles.size()-1;
			int p = possibles[last];
			possibles.set_size(last);

			int dist = my_tree[p].desc.error_of(d);
			if(dist < best_dist) {
				int child = my_tree[p].oldest_child;
				if(child==-1){
					best_dist = dist;
					best_match = p;
				} else {
					while(child!=-1){
						possibles.add(child);
						child = my_tree[child].younger_sibling;
					}
				}
			}
		}
		return best_match;
	}

	inline CVD::ImageRef get_corner(int i){
		return my_tree[i].pos;
	}

	static const int the_max_error=20;

private:
	Container<TreeNode> my_tree;

//	Container<CVD::ImageRef> my_corners;
//	Container<Descriptor> my_descriptors;
//	Container<int> my_first_child;
//	Container<int> my_sibling;
//	Container<int> linked_from;

	Container<int> numpass;

	Container<Merger> mergelist;
//	Container<int> is_merged;
};





#endif
