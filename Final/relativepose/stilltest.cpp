/*
 * stilltest.cpp
 *
 *  Created on: 16/02/2012
 *      Author: winston
 */

#include <../TooN/TooN.h>
#include "pointsetindex.h"
#include "rhips.h"
#include "ransac.h"
#include "epipolar.h"
#include "median.h"

#include <../libcvd/cvd/glwindow.h>
#include <../libcvd/cvd/image_io.h>
#include <../libcvd/cvd/camera.h>
#include <../libcvd/cvd/gl_helpers.h>

#include <vector>
#include <iostream>
#include <sstream>
#include <utility>

int main()
{
  //Load camera parameters
  Camera::Linear camera_model;
  std::ifstream camera_param_path;
  camera_param_path.open("webcamparameters.txt");
  camera_model.load(camera_param_path);
  camera_param_path.close();

  CVD::Image<CVD::byte> img2 = CVD::img_load("im0.ppm");
  CVD::Image<CVD::byte> img1 = CVD::img_load("im8.ppm");

  TooN::Vector<2> const rasterpos1 = TooN::makeVector(0,0);
  TooN::Vector<2> const rasterpos2 = TooN::makeVector(img1.size().x,0);

  CVD::ImageRef windowsize = img1.size();
  windowsize.x = windowsize.x *2;
  CVD::GLWindow display(windowsize);

  std::vector<Descriptor> ref_descriptors;
  std::vector<CVD::ImageRef> ref_unscaled_corners;
  std::vector<TooN::Vector<2> > ref_image_coord_corners;
  std::vector<TooN::Vector<2> > ref_camera_coords_corners;
  std::vector<double> ref_scales;
  std::vector<int> ref_orientations;
  std::vector<Descriptor> ref_r_normalized;

  PointSetIndex ref_psi;
  PointSetIndex ref_psi_r_norm;

  std::vector<Descriptor> live_descriptors;
  std::vector<CVD::ImageRef> live_unscaled_corners;
  std::vector<TooN::Vector<2> > live_image_coord_corners;
  std::vector<TooN::Vector<2> > live_camera_coords_corners;
  std::vector<double> live_scales;
  std::vector<int> live_orientations;
  std::vector<Descriptor> live_r_normalized;
  std::vector<Descriptor> live_rotated;

  std::vector<Ransac<Epipolar> > matches;
  std::vector<std::pair<int,int> > matches_index;
  Hypothesis<Epipolar> current_E;

  while(true)
  {
    ref_descriptors.clear();
    ref_unscaled_corners.clear();
    ref_image_coord_corners.clear();
    ref_camera_coords_corners.clear();
    ref_scales.clear();
    ref_orientations.clear();
    ref_r_normalized.clear();

    // get descriptors of current frame
    live_descriptors.clear();
    live_unscaled_corners.clear();
    live_image_coord_corners.clear();
    live_camera_coords_corners.clear();
    live_scales.clear();
    live_orientations.clear();
    live_r_normalized.clear();
    live_rotated.clear();

    matches.clear();
    matches_index.clear();

    add_image_pyramid(img2, 80, false, live_descriptors, live_unscaled_corners, live_scales);
    add_image_pyramid(img1, 80, false, ref_descriptors, ref_unscaled_corners, ref_scales);

    compute_orientation(live_descriptors,live_orientations);
    compute_orientation(ref_descriptors, ref_orientations);

    compute_normalized_image_coords(live_unscaled_corners, live_scales, live_image_coord_corners);
    compute_normalized_camera_coords(live_image_coord_corners, camera_model, live_camera_coords_corners);
    compute_normalized_image_coords(ref_unscaled_corners, ref_scales, ref_image_coord_corners);
    compute_normalized_camera_coords(ref_image_coord_corners, camera_model, ref_camera_coords_corners);

    std::cout << "got " << ref_descriptors.size() << " points and " << live_descriptors.size() << std::endl;

    rotation_normalize(ref_descriptors, ref_orientations, ref_r_normalized);
    ref_psi_r_norm.build_tree(ref_r_normalized);
    const int global_ori = global_orientation(live_r_normalized,live_orientations, ref_psi_r_norm, ref_orientations, 10);

    ref_psi.build_tree(ref_descriptors);

    std::cout << "Global orientation found " << global_ori << std::endl;

    const int sz = live_descriptors.size();
    for(int i=0; i<sz; i++)
    {
      std::pair<int, int> current_match = ref_psi.best_match(live_descriptors[i].rotate(global_ori), 10);
      if(current_match.second>= 0)
      {
        Ransac<Epipolar> new_match;
        new_match.data.im1 = &(ref_camera_coords_corners[current_match.second]);
        new_match.data.im2 = &(live_camera_coords_corners[i]);
        matches.push_back(new_match);
        matches_index.push_back(std::pair<int, int>(current_match.second, i)); //first:Reference, second:live
      }
    }

    const int matches_sz = matches.size();
    std::vector<TooN::Vector<2> > diff;
    diff.reserve(matches_sz);
    diff.clear();
    for(int i=0; i<matches_sz; i++)
    {
//      std::cout << *(matches[i].data.im1) << " " << *(matches[i].data.im2) << std::endl;
      diff.push_back(*(matches[i].data.im2) - *(matches[i].data.im1) );
    }

    TooN::Vector<3> seedDirection;
    seedDirection.slice<0,2>() = findMedian(diff);
    seedDirection[2] = -0.001;
    TooN::normalize(seedDirection);
    std::cout << "Seed direction: " << seedDirection << std::endl;
    Hypothesis<Epipolar> seedE;
    seedE.Rn = TooN::SO3<> (TooN::makeVector(1,0,0), seedDirection);
    double best_E_score = 0.0;
    ransac(matches, 400, current_E, seedE, best_E_score, 1.0/camera_model.get_parameters()[0]);

    TooN::Matrix<3,3> E = TooN::cross_product_matrix(current_E.Rn.get_matrix() * TooN::makeVector(1,0,0) ) * current_E.Rt.get_matrix();

//    std::cout << current_E.Rn << std::endl;
//    std::cout << current_E.Rt << std::endl;
    std::cout << "Score: " << best_E_score << std::endl;
    std::cout << E << std::endl;
    std::cout << current_E.Rn.get_matrix().T()[0] << " " << current_E.Rt.ln() << std::endl << std::endl;

    display.make_current();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    CVD::glRasterPos(rasterpos1);
    CVD::glDrawPixels(img1);

    CVD::glRasterPos(rasterpos2);
    CVD::glDrawPixels(img2);

    int inliers = 0;
    glColor3f(0,1,0);
    glBegin(GL_LINES);
    for(size_t i = 0; i<matches_index.size(); i++)
    {
      if(matches[i].best_inlier > 0.0)
      {
        CVD::glVertex( ref_image_coord_corners[matches_index[i].first ] );
        CVD::glVertex( live_image_coord_corners[matches_index[i].second ] + rasterpos2 );
        inliers++;
      }
    }
    glEnd();

    std::cout << inliers << " inliers out of " << matches_index.size() << std::endl;

    display.swap_buffers();

    std::cin.get();
  }
}
