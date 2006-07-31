// Copyright (c) 2005  INRIA Sophia-Antipolis (France) 
// All rights reserved.
//
// Authors : Monique Teillaud <Monique.Teillaud@sophia.inria.fr>
//           Sylvain Pion     <Sylvain.Pion@sophia.inria.fr>
//           Julien Hazebrouck
//           Damien Leroy
// 
// Partially supported by the IST Programme of the EU as a Shared-cost
// RTD (FET Open) Project under Contract No  IST-2000-26473 
// (ECG - Effective Computational Geometry for Curves and Surfaces) 
// and a STREP (FET Open) Project under Contract No  IST-006413 
// (ACS -- Algorithms for Complex Shapes)

#ifndef CGAL_ALGEBRAIC_KERNEL_GLOBAL_FUNCTIONS_ON_ROOT_FOR_SPHERE_2_3_H
#define CGAL_ALGEBRAIC_KERNEL_GLOBAL_FUNCTIONS_ON_ROOT_FOR_SPHERE_2_3_H

CGAL_BEGIN_NAMESPACE

template < class AK >
inline 
Comparison_result 
compare_x(const typename AK::Root_for_spheres_2_3& r1,
	   const typename AK::Root_for_spheres_2_3& r2)
{ return AK().compare_x_object()(r1, r2); }

template < class AK >
inline 
Comparison_result 
compare_y(const typename AK::Root_for_spheres_2_3& r1,
	   const typename AK::Root_for_spheres_2_3& r2)
{ return AK().compare_y_object()(r1, r2); }

template < class AK >
inline 
Comparison_result 
compare_z(const typename AK::Root_for_spheres_2_3& r1,
	     const typename AK::Root_for_spheres_2_3& r2)
{ return AK().compare_z_object()(r1, r2); }

template < class AK >
inline 
Comparison_result 
compare_xy(const typename AK::Root_for_spheres_2_3& r1,
	     const typename AK::Root_for_spheres_2_3& r2)
{ return AK().compare_xy_object()(r1, r2); }

template < class AK >
inline 
Comparison_result 
compare_xyz(const typename AK::Root_for_spheres_2_3& r1,
	     const typename AK::Root_for_spheres_2_3& r2)
{ return AK().compare_xyz_object()(r1, r2); }

CGAL_END_NAMESPACE

#endif //CGAL_ALGEBRAIC_KERNEL_GLOBAL_FUNCTIONS_ON_ROOT_FOR_SPHERE_2_3_H
