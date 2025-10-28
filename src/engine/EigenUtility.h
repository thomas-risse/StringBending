#ifndef EIGEN_UTILITY_H
#define EIGEN_UTILITY_H

#include <Eigen/Dense>

template <typename Derived, typename ftype>
auto ClipEigen(const Eigen::MatrixBase<Derived>& array,
          const ftype& min,
          const ftype& max)
{
  return array.cwiseMin(max).cwiseMax(min);
}

#endif // EIGEN_UTILITY_H