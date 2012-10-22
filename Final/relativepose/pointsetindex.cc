#include "pointsetindex.h"

#include <cvd/fast_corner.h>
#include <vector>
#include <cvd/vision.h>
#include <list>
#include <map>

using namespace std;
using namespace CVD;



void PointSetIndex::build_tree(vector<Descriptor>& ps){
  multimap<int,int> unmerged;
  index.reserve(ps.size()*2); // reserve enough for the leaves and the tree above (plus one spare)
  index.resize(ps.size());
  for(int i=0; i<ps.size(); i++){
    index[i].children[0]=index[i].children[1]=-1;
    index[i].imagepointindex=i;
    index[i].descriptor=ps[i];
    unmerged.insert(pair<int,int>(index[i].descriptor.bitcount(),i));	
  }

  typedef multimap<int,int>::iterator It;

  // keep merging nodes until only one left
  while(unmerged.size()>1){
    

    // try to merge the first one in the list
    It merger=unmerged.begin();  // the one we're trying to merge
    It mergee;                  // the best one to merge it with

    bool done=false;
    int bestscore=1000000;

    while(!done){
      // cout << "searching for a close match" << endl;
      done = true;
      Descriptor& d_merger = index[merger->second].descriptor;
      for(It scan = unmerged.begin(); scan != unmerged.end(); scan++){
	if(merger==scan) continue;
	int score = (d_merger | index[scan->second].descriptor).bitcount();
	if(score < bestscore){
	  bestscore = score;

	  mergee=scan; // record this position as it's better than any so far
	  //	  mergee = merger;
	  //	  merger = scan; // look for a better match from here

	  done = false;
	  //	  break;
	}
      }
      std::swap(merger,mergee);
    }
    // cout << "merging" << endl;

    // now merge them!
    DescriptorTree dt;
    dt.descriptor = index[merger->second].descriptor | index[mergee->second].descriptor;
    dt.children[0]=merger->second;
    dt.children[1]=mergee->second;
    dt.imagepointindex=-1;
    index.push_back(dt);
    
    unmerged.erase(merger);
    unmerged.erase(mergee);
    unmerged.insert(pair<int,int>(bestscore,index.size()-1)); // push the new node onto the unmerged list
  }
}

vector<pair<int,int> > PointSetIndex::find_match(const Descriptor& d, int match_threshold){
  vector<pair<int,int> > result;
  vector<int> candidates;
  if(index.size()==0) { // if nothing to do then bail early
    return result;
  }
  candidates.push_back(index.size()-1); // push the root node of the tree onto the candidate list
  while (!candidates.empty()){
    int dtix = candidates.back();
    candidates.pop_back();
    int this_dist= index[dtix].descriptor.error_of(d);
    if(this_dist < match_threshold){
      if(index[dtix].children[0]==-1) { // we've found a leaf
	result.push_back(pair<int,int>(this_dist,index[dtix].imagepointindex));
      } else {
	candidates.push_back(index[dtix].children[0]);
	candidates.push_back(index[dtix].children[1]);
      }
    }
  }
  return result;
}

pair<int,int> PointSetIndex::best_match(const Descriptor& d, int match_threshold){
  int best_dist = match_threshold;
  int best_match = -1;
  vector<int> candidates;
  candidates.reserve(index.size()); // will this speed it up?
  if(index.size() == 0){
    return pair<int,int>(match_threshold,-1);
  }
  candidates.push_back(index.size()-1); // push the root node of the tree onto the candidate list
  
  while (!candidates.empty()){
    int dtix = candidates.back();
    candidates.pop_back();
    int this_dist= index[dtix].descriptor.error_of(d);
    if(this_dist < best_dist){
      if(index[dtix].children[0]==-1) { // we've found a leaf
	best_dist = this_dist;
	best_match = index[dtix].imagepointindex;
      } else {
	candidates.push_back(index[dtix].children[0]);
	  candidates.push_back(index[dtix].children[1]);
      }
    }
  }
  return pair<int,int>(best_dist,best_match);
}


