// -*- c++ -*-

#ifndef RANSAC_H
#define RANSAC_H

#include <vector>
#include <stdlib.h>
#include <TooN/TooN.h>

template <class T>
class Ransac {
 public:
  T data;  // the payload used to generate and test hypotheses
  int match_error;

  double this_inlier;
  double best_inlier;
};

// replace this with your own class with these functions and any additional data needed
template <class T>
class Hypothesis {
 public:
  void generate(const std::vector<Ransac<T>* >& gen_set);
  double is_inlier(const Ransac<T>& test);
};

template<class T>
void ransac(std::vector<Ransac<T> >& samples, int const num_iterations, Hypothesis<T>& hyp, Hypothesis<T>& prev_hyp, double& best_score, double const inlier_threshold){

  const int sample_size = T::sample_size;
  const size_t num_measurements = samples.size();


  //check with previous hypothesis
  double inlier_score=0;
  for(size_t i=0; i<num_measurements; i++){
    double score = prev_hyp.is_inlier(samples[i], inlier_threshold);
    samples[i].this_inlier = score;
    inlier_score += score;
  }
    

  // std::cout << "previous hypothesis consensus score calculated" << std::endl;

  if(inlier_score > best_score){
    best_score = inlier_score;
    hyp = prev_hyp;
    for(size_t i=0; i<num_measurements; i++){
        samples[i].best_inlier = samples[i].this_inlier;
    }
  }

  for(int iter=0; iter<num_iterations; iter++)
  {

    // std::cout << iter << std::endl;

    // draw random samples without replacement
    // draw sample_size indexes from the list
    size_t sample_nos[sample_size];
    for(int i=0; i<sample_size; i++){
      sample_nos[i] = ((unsigned int)lrand48())%(num_measurements-i);
      for(int j=0; j<i; j++)
      {
        if(sample_nos[i] >= sample_nos[j])
        {
          sample_nos[i]++;
        }
        else
        {
          std::swap(sample_nos[i],sample_nos[j]);
        }
      }
    }

    // now populate the gen_set (from which the hypothesis will be generated) from the sample indexes
    std::vector<Ransac<T>* > gen_set(sample_size);
    for(int i=0; i<sample_size; i++)
    {
      gen_set[i]=&(samples[sample_nos[i]]);
    }

    // std::cout << "gen set populated" << std::endl;

    // generate a hypothesis
    Hypothesis<T> hypothesis;
    hypothesis.generate(gen_set);

    // std::cout << "hypothesis generated" << std::endl;

    // calculate inlier score;
    inlier_score=0;
    for(int i=0; i<num_measurements; i++)
    {
      double score = hypothesis.is_inlier(samples[i], inlier_threshold);
      samples[i].this_inlier = score;
      inlier_score += score;
    }
    // std::cout << "consensus score calculated" << std::endl;


    if(inlier_score > best_score)
    {
      best_score = inlier_score;
      hyp = hypothesis;
      for(int i=0; i<num_measurements; i++)
      {
        samples[i].best_inlier = samples[i].this_inlier;
      }
    }

  }

  // now have the best inlier set for these iterations - so optimise it

  bool done=false;
  while(!done)
  {
    done=true;
    // best_score = total_score;
    // hyp=hyp2;
    double total_score;
    Hypothesis<T> hypothesis;

    // optimise using all the inliers
    std::vector<Ransac<T>*> inliers;
    for(unsigned int i=0; i<samples.size(); i++){
      if(samples[i].best_inlier > 0){
	inliers.push_back(&samples[i]);
      }
    }
    hypothesis.generate(inliers);

    total_score = 0;
    for(unsigned int i=0; i<samples.size(); i++)
    {
      double score =  hypothesis.is_inlier(samples[i],inlier_threshold);
      samples[i].this_inlier = score;
      total_score += score;
    }
    if(total_score > best_score)
    {
      hyp=hypothesis;
      best_score=total_score;
      for(unsigned int i=0; i<samples.size(); i++) {
        samples[i].best_inlier = samples[i].this_inlier;
      }
      done=false;
    }

  }

}



#endif
