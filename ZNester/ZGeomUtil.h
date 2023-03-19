#pragma once
#include <cmath>

constexpr double   DBL_TOL			 = 1e-4;
constexpr double   DEG2_RAD			 = 0.017453293;
constexpr double   RAD_2DEG			 = 57.2957796;
constexpr double   CLIPPER_LIB_SCALE = 1048576.0;

__forceinline bool dblEqual( const double& a, const double& b, const double& tolerance = DBL_TOL )
{
	return abs( a - b ) < tolerance;
}
__forceinline bool dblSmaller( const double& a, const double& b, const double& tolerance = DBL_TOL )
{
	return !dblEqual( a, b, tolerance ) && a < b;
}
__forceinline bool dblLarger( const double& a, const double& b, const double& tolerance = DBL_TOL )
{
	return !dblEqual( a, b, tolerance ) && a > b;
}
