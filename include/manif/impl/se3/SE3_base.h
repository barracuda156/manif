#ifndef _MANIF_MANIF_SE3_BASE_H_
#define _MANIF_MANIF_SE3_BASE_H_

#include "manif/impl/se3/SE3_properties.h"
#include "manif/impl/manifold_base.h"
#include "manif/impl/so3/SO3_map.h"

namespace manif
{

////////////////
///          ///
/// Manifold ///
///          ///
////////////////

template <typename _Derived>
struct SE3Base : ManifoldBase<_Derived>
{
private:

  using Base = ManifoldBase<_Derived>;
  using Type = SE3Base<_Derived>;

public:

  MANIF_MANIFOLD_PROPERTIES

  MANIF_MANIFOLD_TYPEDEF

  /// @todo find a mechanism to fetch it from base
  /// just like the other typedefs
  using Translation = typename internal::traits<_Derived>::Translation;

  /// Manifold common API

  Transformation transform() const;
  Rotation rotation() const;
  Translation translation() const;

  SE3Base<_Derived>& setIdentity();

  Manifold inverse(OptJacobianRef J_minv_m = {}) const;
  Tangent lift(OptJacobianRef J_t_m = {}) const;

  template <typename _DerivedOther>
  Manifold compose(const ManifoldBase<_DerivedOther>& m,
                   OptJacobianRef J_mc_ma = {},
                   OptJacobianRef J_mc_mb = {}) const;

  Vector act(const Vector &v,
             OptJacobianRef J_vout_m = {},
             OptJacobianRef J_vout_v = {}) const;

  Jacobian adj() const;

  using Base::coeffs;
  using Base::coeffs_nonconst;
  using Base::data;
  MANIF_INHERIT_MANIFOLD_AUTO_API
  MANIF_INHERIT_MANIFOLD_OPERATOR

  /// SE3 specific functions

  Scalar x() const;
  Scalar y() const;
  Scalar z() const;

  //Scalar roll() const;
  //Scalar pitch() const;
  //Scalar yaw() const;

//protected:

  /// Helper

//  auto trapart()
//  -> decltype( std::declval<Type>().coeffs().template block<Dim, 1>(0,0) )
//  {
//    coeffs().block<Dim, 1>(0,0);
//  }

//  auto trapart() const
//  -> decltype( std::declval<const Type>().coeffs().template block<Dim, 1>(0,0) )
//  {
//    coeffs().block<Dim, 1>(0,0);
//  }

//  auto rotpart() const
//  -> decltype( std::declval<const Type>().coeffs().template block<4, 1>(3,0) )
//  {
//    coeffs().block<4, 1>(3,0);
//  }

//  auto rotpart() const
//  -> decltype( std::declval<const Type>().coeffs().template block<4, 1>(3,0) )
//  {
//    coeffs().block<4, 1>(3,0);
//  }

  Eigen::Map<const SO3<Scalar>> asSO3() const
  {
    return Eigen::Map<const SO3<Scalar>>(coeffs().data()+3);
  }

  Eigen::Map<SO3<Scalar>> asSO3()
  {
    return Eigen::Map<SO3<Scalar>>(coeffs_nonconst().data()+3);
  }
};

template <typename _Derived>
typename SE3Base<_Derived>::Transformation
SE3Base<_Derived>::transform() const
{
  Transformation T = Transformation::Identity();
  T.template topLeftCorner<3,3>() = rotation();
  T(0,3) = x();
  T(1,3) = y();
  T(2,3) = z();
  return T;
}

template <typename _Derived>
typename SE3Base<_Derived>::Rotation
SE3Base<_Derived>::rotation() const
{
  return asSO3().rotation();
}

template <typename _Derived>
typename SE3Base<_Derived>::Translation
SE3Base<_Derived>::translation() const
{
  return coeffs().template head<3>();
}

template <typename _Derived>
SE3Base<_Derived>&
SE3Base<_Derived>::setIdentity()
{
  coeffs_nonconst().setZero();
  asSO3().setIdentity();
  return *this;
}

template <typename _Derived>
typename SE3Base<_Derived>::Manifold
SE3Base<_Derived>::inverse(OptJacobianRef J_minv_m) const
{
  if (J_minv_m)
  {
    /// @note
    ///
    /// J = | R^T  -R u_x t
    ///     |  0       I
    ///

    static const Eigen::Matrix<Scalar,Dim,Dim> u_x(
         ( Eigen::Matrix<Scalar,Dim,Dim>() <<
            Scalar(0), -Scalar(1),  Scalar(1),
            Scalar(1),  Scalar(0), -Scalar(1),
           -Scalar(1),  Scalar(1),  Scalar(0) ).finished()
          );

    J_minv_m->setIdentity();
    J_minv_m->template topLeftCorner<Dim,Dim>() =
        rotation().transpose();
    J_minv_m->template topRightCorner<Dim,1>() =
        J_minv_m->template topLeftCorner<Dim,Dim>() * u_x * translation();
  }

  return Manifold(-rotation().transpose() * translation(),
                   asSO3().inverse().quat());
}

template <typename _Derived>
typename SE3Base<_Derived>::Tangent
SE3Base<_Derived>::lift(OptJacobianRef J_t_m) const
{
  using std::abs;
  using std::sqrt;

//  using SO3Lie = typename SO3Tangent<Scalar>::LieType;

  const SO3Tangent<Scalar> so3tan = asSO3().lift();
/*
  const SO3Lie W = so3tan.hat();

  const Scalar theta_sq = so3tan.coeffs().squaredNorm();
  const Scalar theta = sqrt( theta_sq );

  Scalar A,  // sin_theta_by_theta
         B;  // one_minus_cos_theta_by_theta_squared

  if (abs(theta) < Constants<Scalar>::eps)
  {
    // Taylor approximation
    A = Scalar(1.) - Scalar(1. / 6.)  * theta_sq;
    B = Scalar(1. / 2.) - Scalar(1. / 24.) * theta_sq;
  }
  else
  {
    // Euler
    A = sin(theta) /  theta;
    B = (Scalar(1) - cos(theta)) / theta*theta;
  }

  const SO3Lie Vinv = SO3Lie::Identity() -
      Scalar(1./2.) * W +
      Scalar(1./theta_sq) * (Scalar(1) - A/(Scalar(2)*B)) * (W*W);

  if (J_t_m)
  {
    MANIF_NOT_IMPLEMENTED_YET;
  }

  return Tangent((typename Tangent::DataType() <<
                  Vinv*translation(), so3tan.coeffs()).finished());
*/

  if (J_t_m)
  {
    MANIF_NOT_IMPLEMENTED_YET;
  }

  return Tangent((typename Tangent::DataType() <<
                  so3tan.ljac().inverse()*translation(),
                  so3tan.coeffs()).finished());
}

template <typename _Derived>
template <typename _DerivedOther>
typename SE3Base<_Derived>::Manifold
SE3Base<_Derived>::compose(
    const ManifoldBase<_DerivedOther>& m,
    OptJacobianRef J_mc_ma,
    OptJacobianRef J_mc_mb) const
{
  static_assert(
    std::is_base_of<SE3Base<_DerivedOther>, _DerivedOther>::value,
    "Argument does not inherit from SE3Base !");

  const auto& m_se3 = static_cast<const SE3Base<_DerivedOther>&>(m);

  if (J_mc_ma)
  {
    /// @note
    ///
    /// J = | I  -R tb_x
    ///     | 0    Rb^T
    ///

    J_mc_ma->setIdentity();

    J_mc_ma->template bottomRightCorner<Dim,Dim>() =
        m_se3.rotation().transpose();

    J_mc_ma->template topRightCorner<Dim,Dim>() =
        -rotation() * skew3(translation());
  }

  if (J_mc_mb)
  {
    /// @note
    ///
    /// J = | R 0
    ///     | 0 I
    ///

    J_mc_mb->setIdentity();
    J_mc_mb->template topLeftCorner<Dim,Dim>() = rotation();

    /// @note
    ///
    /// J = | R  t_x R
    ///     | 0    R
    ///
/*
    J_mc_mb->setIdentity();

    J_mc_mb->template topLeftCorner<Dim,Dim>() = rotation();

    J_mc_mb->template bottomRightCorner<Dim,Dim>() =
        J_mc_mb->template topLeftCorner<Dim,Dim>();

    J_mc_mb->template topRightCorner<Dim,1>() =
        skew(translation()) * J_mc_mb->template topLeftCorner<Dim,Dim>();
*/
  }

  return Manifold(rotation()*m_se3.translation() + translation(),
                  asSO3().compose(m_se3.asSO3()).quat());
}

template <typename _Derived>
typename SE3Base<_Derived>::Vector
SE3Base<_Derived>::act(const Vector &v,
                       OptJacobianRef J_vout_m,
                       OptJacobianRef J_vout_v) const
{
  if (J_vout_m)
  {
    MANIF_NOT_IMPLEMENTED_YET
  }

  if (J_vout_v)
  {
    MANIF_NOT_IMPLEMENTED_YET
  }

  return transform() * v;
}

template <typename _Derived>
typename SE3Base<_Derived>::Jacobian
SE3Base<_Derived>::adj() const
{
  Rotation T;
  T <<  Scalar(0), -z()      ,  y(),
        z()      ,  Scalar(0), -x(),
       -y()      ,  x()      ,  Scalar(0);

  Jacobian Adj = Jacobian::Zero();
  Adj.template topLeftCorner<3,3>() = rotation();
  Adj.template bottomRightCorner<3,3>() =
      Adj.template topLeftCorner<3,3>();
  Adj.template bottomLeftCorner<3,3>() =
    T * Adj.template topLeftCorner<3,3>();

  return Adj;
}

/// SE3 specific function

template <typename _Derived>
typename SE3Base<_Derived>::Scalar
SE3Base<_Derived>::x() const
{
  return coeffs().x();
}

template <typename _Derived>
typename SE3Base<_Derived>::Scalar
SE3Base<_Derived>::y() const
{
  return coeffs().y();
}

template <typename _Derived>
typename SE3Base<_Derived>::Scalar
SE3Base<_Derived>::z() const
{
  return coeffs().z();
}

} /* namespace manif */

#endif /* _MANIF_MANIF_SE3_BASE_H_ */
