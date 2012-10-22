#include "epipolar.h"

#define TOON_DEBUG 1


#include <TooN/TooN.h>
#include <TooN/wls.h>
#include <TooN/irls.h>
#include <utility>

#include <iomanip>

//#include <cvd/random.h>
namespace {
double sq(double x)
{
  return x*x;
}

// Define a line l = [a b c] so that [x y 1] . [a b c] = 0
//
// The line equation can be written as:
//
// y = -ax/b -c/b
//
// And written in vector form:
//
// [ x ]   [0   ]              [ b ]
// [ y ] = [-c/b]  + x (1/b) * [ -a]
//
//The line normal therefore is v = [a b]
//
// r is the vector from [x0 y0] to any point on the line. The perpendicular diatance to the
// line is |r . v| / |v|
//
// d = | (x - x0)*a + (y-y0) * b | / sqrt(a^2 + b^2)
//
// Rearranging and using ax + yb = -c:
//
// d = |ax0 + by0 + c| / sqrt(a*a + b*b)
double point_line_distance_squared(TooN::Vector<3> point, const TooN::Vector<3>& line)
{
  //Normalize the point to [x0, y0, 1]
  point /= point[2];

  return sq(point * line) / (sq(line[0]) + sq(line[1]));
}

double point_line_distance(TooN::Vector<3> point, const TooN::Vector<3>& line)
{
  //Normalize the point to [x0, y0, 1]
  point /= point[2];

  return point * line / sqrt(sq(line[0]) + sq(line[1]));
}


std::pair<double, double> essential_reprojection_errors_squared(const TooN::Matrix<3>& E, const TooN::Vector<3>& q, const TooN::Vector<3>& p)
{
  return std::make_pair(point_line_distance_squared(p, E*q), point_line_distance_squared(q, E.T()*p));
}

std::pair<double, double> essential_reprojection_errors(const TooN::Matrix<3>& E, const TooN::Vector<3>& q, const TooN::Vector<3>& p)
{
  return std::make_pair(point_line_distance(p, E*q), point_line_distance(q, E.T()*p));
}

}

void Hypothesis<Epipolar>::generate(const std::vector<Ransac<Epipolar>* >& gen_set){
  TooN::Vector<3> X = TooN::makeVector(1,0,0);
//  TooN::Vector<3> trans = TooN::unit(TooN::makeVector(CVD::rand_g(), CVD::rand_g(), CVD::rand_g()));
  TooN::Vector<3> trans = TooN::makeVector(1,0,0);

  // guess rotation between cameras is nothing
  // Rt = Identity; // not needed because default initialiser is identity.
  Rn = TooN::SO3<> (X,trans);

  int count = 0;
  double old_err =  HUGE_VAL;
  TooN::WLS<5, double> wls;
  do {
    // cout << "count " << count << endl;
    wls.clear();
    double err=0;
    const TooN::Matrix<3> C = TooN::cross_product_matrix(Rn * X);
    for(size_t i = 0; i < gen_set.size(); ++i){
      // cout << "gen set point " << i << endl;
      TooN::Vector<5> J;

      // cout << gen_set[i]->data.im1 << endl;
      // cout << gen_set[i]->data.im2 << endl;


      TooN::Vector<3> v1 = TooN::unproject( *(gen_set[i]->data.im1));
      TooN::Vector<3> v2 = TooN::unproject( *(gen_set[i]->data.im2));

      // cout << "copied v1 and v2 from gen_set[" << i << "]" << endl;

      const TooN::Vector<3> LEFT = v2 * C;
      const TooN::Vector<3> RIGHT = Rt * v1;

      // cout << "about to build Jacobian" << endl;

      J[0] = LEFT * Rt.generator_field(0, RIGHT);
      J[1] = LEFT * Rt.generator_field(1, RIGHT);
      J[2] = LEFT * Rt.generator_field(2, RIGHT);
      J[3] = v2 * TooN::cross_product_matrix(Rn * Rn.generator_field(1, X)) * RIGHT;
      J[4] = v2 * TooN::cross_product_matrix(Rn * Rn.generator_field(2, X)) * RIGHT;

      // cout << "jacobian built" << endl;

      const double e = 0 - LEFT * RIGHT;
      err += e * e;
      wls.add_mJ(e, J);

      // cout << "added to wls" << endl;
    }

    // cout << "points added to wls" << endl;

    if(err>old_err)
      break;
    old_err = err;
    wls.add_prior(1e-6);
    wls.compute();

    // cout << "wls computed" << endl;

    Rt = TooN::SO3<>::exp(wls.get_mu().slice<0,3>()) * Rt;
    Rn = Rn * TooN::SO3<>::exp(TooN::makeVector(0, wls.get_mu()[3], wls.get_mu()[4]));

    ++count;
  } while(TooN::norm_sq(wls.get_mu()) > 1e-6 && count < 6);
}

double Hypothesis<Epipolar>::is_inlier(const Ransac<Epipolar>& test, double threshold){
  TooN::Vector<3> X = TooN::makeVector(1,0,0);
  TooN::Matrix<3> E = TooN::cross_product_matrix(Rn * X) * Rt;

  TooN::Vector<3> v1 = TooN::unproject( *(test.data.im1));
  TooN::Vector<3> v2 = TooN::unproject( *(test.data.im2));

//  TooN::Vector<3> rline = v2*E;
//  rline /= std::sqrt(rline[0]*rline[0] + rline[1]*rline[1]);
//
//  TooN::Vector<3> lline = E*v1;
//  lline /= std::sqrt(lline[0]*lline[0] + lline[1]*lline[1]);
//
//  double lerr = v2*lline;
//  double rerr = rline*v1;
  std::pair<double,double> err = essential_reprojection_errors_squared(E, v2, v1);

  double errsq = err.first + err.second;

  double score = 1 - errsq/(threshold*threshold);
  if(score < 0){
    score=0;
  }

  return score;
}

void  Hypothesis<Epipolar>::save(std::ostream& os){
  os << std::setprecision(10) << Rt << std::endl << Rn << std::endl;
}

void  Hypothesis<Epipolar>::load(std::istream& is){
  is >> Rt >> Rn;
}


