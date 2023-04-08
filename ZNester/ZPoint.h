#pragma once
#include <cassert>
#include <cfloat>
#include <cmath>

#include "ZGeomUtil.h"

class ZPoint
{
public:
	__forceinline ZPoint() = default;
	__forceinline ZPoint( double x, double y )
		: m_x( x )
		, m_y( y )
	{
	}
	__forceinline ZPoint( const ZPoint& p )			   = default;
	__forceinline ZPoint( ZPoint&& p )				   = default;
	__forceinline ~ZPoint()							   = default;
	__forceinline ZPoint& operator=( ZPoint&& p )	   = default;
	__forceinline ZPoint& operator=( const ZPoint& p ) = default;

	__forceinline double  x() const
	{
		return m_x;
	};
	__forceinline double y() const
	{
		return m_y;
	}

	__forceinline void setX( double x )
	{
		m_x = x;
	}
	__forceinline void setY( double y )
	{
		m_y = y;
	}

	__forceinline bool marked() const
	{
		return m_marked;
	}
	__forceinline void setMarked( bool b )
	{
		m_marked = b;
	}

	__forceinline bool isValid() const
	{
		return m_x != DBL_MAX && m_y != DBL_MAX;
	}

	__forceinline double dot( const ZPoint& b ) const
	{
		return m_x * b.m_x + m_y * b.m_y;
	}
	__forceinline double cross( const ZPoint& b ) const
	{
		return m_x * b.m_y - m_y * b.m_x;
	}
	__forceinline ZPoint normal() const
	{
		return ZPoint( m_y, -m_x );
	}

	__forceinline double length() const
	{
		return std::sqrt( m_x * m_x + m_y * m_y );
	}
	__forceinline double length2() const
	{
		return m_x * m_x + m_y * m_y;
	}
	__forceinline ZPoint normalized() const
	{
		double l = length();
		return ZPoint( m_x / l, m_y / l );
	}
	__forceinline double distance( const ZPoint& b ) const
	{
		return ZPoint( m_x - b.m_x, m_y - b.m_y ).length();
	}
	__forceinline double det( const ZPoint& b ) const
	{
		return m_x * b.m_y - m_y * b.m_x;
	}
	__forceinline double angle() const
	{
		return ( std::atan2( m_x, m_y ) ) * RAD_2DEG;
	}
	__forceinline bool			onSegment( const ZPoint& a, const ZPoint& b ) const;

	__forceinline static double pointDistance( const ZPoint& p, const ZPoint& s1, const ZPoint& s2, const ZPoint& norm,
											   bool infinite = false );
	__forceinline static double segmentDistance( const ZPoint& a, const ZPoint& b, const ZPoint& e, const ZPoint& f,
												 const ZPoint& direction );
	__forceinline static ZPoint lineIntersect( const ZPoint& a, const ZPoint& b, const ZPoint& e, const ZPoint& f,
											   bool infinite = false );
	__forceinline static double ZCrossProduct( const ZPoint& k, const ZPoint& k1, const ZPoint& k2 );

	__forceinline ZPoint		operator*( const double& a ) const
	{
		return ZPoint( m_x * a, m_y * a );
	}
	__forceinline ZPoint operator-() const
	{
		return ZPoint( -m_x, -m_y );
	}

	__forceinline ZPoint& operator+=( const ZPoint& b )
	{
		m_x += b.m_x;
		m_y += b.m_y;
		return *this;
	}
	__forceinline ZPoint& operator+=( const double& b )
	{
		m_x += b;
		m_y += b;
		return *this;
	}
	__forceinline ZPoint& operator-=( const ZPoint& b )
	{
		m_x -= b.m_x;
		m_y -= b.m_y;
		return *this;
	}
	__forceinline ZPoint& operator-=( const double& b )
	{
		m_x -= b;
		m_y -= b;
		return *this;
	}
	__forceinline ZPoint& operator*=( const double& b )
	{
		m_x *= b;
		m_y *= b;
		return *this;
	}
	__forceinline ZPoint& operator/=( const double& b )
	{
		m_x /= b;
		m_y /= b;
		return *this;
	}

	__forceinline bool operator==( const ZPoint& b ) const
	{
		return dblEqual( m_x, b.m_x ) && dblEqual( m_y, b.m_y );
	}
	__forceinline bool operator==( const double& d ) const
	{
		return dblEqual( m_x, d ) && dblEqual( m_y, d );
	}
	__forceinline bool operator!=( const ZPoint& b ) const
	{
		return !dblEqual( m_x, b.m_x ) || !dblEqual( m_y, b.m_y );
	}
	__forceinline bool operator<( const ZPoint& b ) const
	{
		if ( !dblEqual( m_x, b.m_x ) )
			return m_x < b.m_x;
		if ( !dblEqual( m_y, b.m_y ) )
			return m_y < b.m_y;
		return false;
	}

private:
	double m_x		= DBL_MAX;
	double m_y		= DBL_MAX;
	bool   m_marked = false;
};

__forceinline ZPoint operator+( const ZPoint& a, const ZPoint& b )
{
	return ZPoint( a.x() + b.x(), a.y() + b.y() );
}
__forceinline ZPoint operator+( const ZPoint& a, const double& b )
{
	return ZPoint( a.x() + b, a.y() + b );
}
__forceinline ZPoint operator+( const double& a, const ZPoint& b )
{
	return ZPoint( a + b.x(), a + b.y() );
}
__forceinline ZPoint operator-( const ZPoint& a, const ZPoint& b )
{
	return ZPoint( a.x() - b.x(), a.y() - b.y() );
}
__forceinline ZPoint operator-( const ZPoint& a, const double& b )
{
	return ZPoint( a.x() - b, a.y() - b );
}
__forceinline ZPoint operator-( const double& a, const ZPoint& b )
{
	return ZPoint( a - b.x(), a - b.y() );
}
__forceinline ZPoint operator*( const ZPoint& a, const ZPoint& b )
{
	return ZPoint( a.x() * b.x(), a.y() * b.y() );
}
__forceinline ZPoint operator*( const double& a, const ZPoint& b )
{
	return ZPoint( a * b.x(), a * b.y() );
}
__forceinline ZPoint operator/( const ZPoint& a, const ZPoint& b )
{
	return ZPoint( a.x() / b.x(), a.y() / b.y() );
}
__forceinline ZPoint operator/( const ZPoint& a, const double& b )
{
	return ZPoint( a.x() / b, a.y() / b );
}
__forceinline ZPoint operator/( const double& a, const ZPoint& b )
{
	return ZPoint( a / b.x(), a / b.y() );
}

__forceinline bool ZPoint::onSegment( const ZPoint& a, const ZPoint& b ) const
{
	// vertical line
	if ( dblEqual( a.m_x, b.m_x ) && dblEqual( m_x, a.m_x ) )
	{
		if ( !dblEqual( m_y, b.m_y ) && !dblEqual( m_y, a.m_y ) && m_y < std::max( b.m_y, a.m_y ) &&
			 m_y > std::min( b.m_y, a.m_y ) )
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	// horizontal line
	if ( dblEqual( a.m_y, b.m_y ) && dblEqual( m_y, a.m_y ) )
	{
		if ( !dblEqual( m_x, b.m_x ) && !dblEqual( m_x, a.m_x ) && m_x < std::max( b.m_x, a.m_x ) &&
			 m_x > std::min( b.m_x, a.m_x ) )
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	// range check
	if ( ( m_x < a.m_x && m_x < b.m_x ) || ( m_x > a.m_x && m_x > b.m_x ) || ( m_y < a.m_y && m_y < b.m_y ) ||
		 ( m_y > a.m_y && m_y > b.m_y ) )
	{
		return false;
	}

	// exclude end points
	if ( ( dblEqual( m_x, a.m_x ) && dblEqual( m_y, a.m_y ) ) || ( dblEqual( m_x, b.m_x ) && dblEqual( m_y, b.m_y ) ) )
	{
		return false;
	}

	auto cross = ( m_y - a.m_y ) * ( b.m_x - a.m_x ) - ( m_x - a.m_x ) * ( b.m_y - a.m_y );

	if ( fabs( cross ) > DBL_TOL )
	{
		return false;
	}

	auto dot = ( m_x - a.m_x ) * ( b.m_x - a.m_x ) + ( m_y - a.m_y ) * ( b.m_y - a.m_y );

	if ( dot < 0 || dblEqual( dot, 0 ) )
	{
		return false;
	}

	// auto len2 = ( b - a ).length2();
	auto len2 = ( b.m_x - a.m_x ) * ( b.m_x - a.m_x ) + ( b.m_y - a.m_y ) * ( b.m_y - a.m_y );

	if ( dot > len2 || dblEqual( dot, len2 ) )
	{
		return false;
	}

	return true;
}

__forceinline double ZPoint::pointDistance( const ZPoint& p, const ZPoint& s1, const ZPoint& s2, const ZPoint& norm,
											bool infinite )
{
	auto   normVec	 = norm;
	ZPoint dir		 = normVec.normal();

	auto   pDot		 = p.dot( dir );
	auto   s1Dot	 = s1.dot( dir );
	auto   s2Dot	 = s2.dot( dir );

	auto   pDotNorm	 = p.dot( normVec );
	auto   s1DotNorm = s1.dot( normVec );
	auto   s2DotNorm = s2.dot( normVec );

	if ( !infinite )
	{
		auto dots1dot = dblEqual( pDot, s1Dot );
		auto dots2dot = dblEqual( pDot, s2Dot );
		if ( ( ( pDot < s1Dot || dots1dot ) && ( pDot < s2Dot || dots2dot ) ) ||
			 ( ( pDot > s1Dot || dots1dot ) && ( pDot > s2Dot || dots2dot ) ) )
		{
			return DBL_MAX;	 // dot doesn't collide with segment, or lies directly on the vertex
		}
		if ( ( dots1dot && dots2dot ) && ( pDotNorm > s1DotNorm && pDotNorm > s2DotNorm ) )
		{
			return std::min( pDotNorm - s1DotNorm, pDotNorm - s2DotNorm );
		}
		if ( ( dots1dot && dots2dot ) && ( pDotNorm < s1DotNorm && pDotNorm < s2DotNorm ) )
		{
			return -std::min( s1DotNorm - pDotNorm, s2DotNorm - pDotNorm );
		}
	}

	return -( pDotNorm - s1DotNorm + ( s1DotNorm - s2DotNorm ) * ( s1Dot - pDot ) / ( s1Dot - s2Dot ) );
}
__forceinline double ZPoint::segmentDistance( const ZPoint& a, const ZPoint& b, const ZPoint& e, const ZPoint& f,
											  const ZPoint& direction )
{
	ZPoint normal = direction.normal();

	auto   dotA	  = a.dot( normal );
	auto   dotB	  = b.dot( normal );
	auto   dotE	  = e.dot( normal );
	auto   dotF	  = f.dot( normal );

	auto   abMin  = std::min( dotA, dotB );
	auto   abMax  = std::max( dotA, dotB );

	auto   efMax  = std::max( dotE, dotF );
	auto   efMin  = std::min( dotE, dotF );

	// segments miss each other completely
	if ( abMax < efMin || abMin > efMax )
	{
		return DBL_MAX;
	}
	// segments that will merely touch at one point
	if ( dblEqual( abMax, efMin ) || dblEqual( abMin, efMax ) )
	{
		return DBL_MAX;
	}

	auto crossAbe = ( e.m_y - a.m_y ) * ( b.m_x - a.m_x ) - ( e.m_x - a.m_x ) * ( b.m_y - a.m_y );
	auto crossAbf = ( f.m_y - a.m_y ) * ( b.m_x - a.m_x ) - ( f.m_x - a.m_x ) * ( b.m_y - a.m_y );

	// lines are colinear
	if ( dblEqual( crossAbe, 0 ) && dblEqual( crossAbf, 0 ) )
	{
		ZPoint abNorm = ( b - a ).normal().normalized();
		ZPoint efNorm = ( f - e ).normal().normalized();

		// segment normals must point in opposite directions
		if ( std::abs( efNorm.cross( abNorm ) ) < DBL_TOL && efNorm.dot( abNorm ) < DBL_TOL )
		{
			// normal of AB segment must point in same direction as given direction vector
			auto normDot = abNorm.dot( direction );
			// the segments merely slide along each other
			if ( dblEqual( normDot, 0 ) )
			{
				return DBL_MAX;
			}
			if ( normDot < 0 )
			{
				return 0;
			}
		}
		return DBL_MAX;
	}

	auto distance = DBL_MAX;
	auto overlap  = 0.0;
	if ( ( abMax > efMax && abMin < efMin ) || ( efMax > abMax && efMin < abMin ) )
	{
		overlap = 1.0;
	}
	else
	{
		auto minMax = std::min( abMax, efMax );
		auto maxMin = std::max( abMin, efMin );

		auto maxMax = std::max( abMax, efMax );
		auto minMin = std::min( abMin, efMin );

		overlap		= ( minMax - maxMin ) / ( maxMax - minMin );
	}

	ZPoint reverse = -direction;

	auto   crossA  = a.dot( direction );
	auto   crossE  = e.dot( direction );
	auto   crossF  = f.dot( direction );

	// coincident points
	if ( dblEqual( dotA, dotE ) )
	{
		distance = crossA - crossE;
	}
	else if ( dblEqual( dotA, dotF ) )
	{
		distance = crossA - crossF;
	}
	else if ( dotA > efMin && dotA < efMax )
	{
		auto d = ZPoint::pointDistance( a, e, f, reverse );
		if ( dblEqual( d, 0 ) )
		{
			//  A currently touches EF, but AB is moving away from EF
			auto dB = ZPoint::pointDistance( b, e, f, reverse, true );
			if ( dB < 0 || dblEqual( dB * overlap, 0 ) )
			{
				d = DBL_MAX;
			}
		}
		if ( d != DBL_MAX )
		{
			distance = d;
		}
	}

	auto crossB = b.dot( direction );

	if ( dblEqual( dotB, dotE ) )
	{
		auto dist = crossB - crossE;
		if ( dist < distance )
			distance = dist;
	}
	else if ( dblEqual( dotB, dotF ) )
	{
		auto dist = crossB - crossF;
		if ( dist < distance )
			distance = dist;
	}
	else if ( dotB > efMin && dotB < efMax )
	{
		auto d = ZPoint::pointDistance( b, e, f, reverse );

		if ( dblEqual( d, 0 ) )
		{
			// crossA>crossB A currently touches EF, but AB is moving away from EF
			auto dA = ZPoint::pointDistance( a, e, f, reverse, true );
			if ( dA < 0 || dblEqual( dA * overlap, 0 ) )
			{
				d = DBL_MAX;
			}
		}
		if ( d < distance )
			distance = d;
	}

	if ( dotE > abMin && dotE < abMax )
	{
		auto d = ZPoint::pointDistance( e, a, b, direction );
		if ( dblEqual( d, 0 ) )
		{
			// crossF<crossE A currently touches EF, but AB is moving away from EF
			auto dF = ZPoint::pointDistance( f, a, b, direction, true );
			if ( dF < 0 || dblEqual( dF * overlap, 0 ) )
			{
				d = DBL_MAX;
			}
		}
		if ( d < distance )
			distance = d;
	}

	if ( dotF > abMin && dotF < abMax )
	{
		auto d = ZPoint::pointDistance( f, a, b, direction );
		if ( dblEqual( d, 0 ) )
		{
			// crossE<crossF A currently touches EF, but AB is moving away from EF
			auto dE = ZPoint::pointDistance( e, a, b, direction, true );
			if ( dE < 0 || dblEqual( dE * overlap, 0 ) )
			{
				d = DBL_MAX;
			}
		}
		if ( d < distance )
			distance = d;
	}

	return distance;
}
__forceinline ZPoint ZPoint::lineIntersect( const ZPoint& a, const ZPoint& b, const ZPoint& e, const ZPoint& f,
											bool infinite )
{
	auto   a1	 = b.m_y - a.m_y;
	auto   b1	 = a.m_x - b.m_x;
	auto   c1	 = b.m_x * a.m_y - a.m_x * b.m_y;
	auto   a2	 = f.m_y - e.m_y;
	auto   b2	 = e.m_x - f.m_x;
	auto   c2	 = f.m_x * e.m_y - e.m_x * f.m_y;

	double denom = a1 * b2 - a2 * b1;
	if ( dblEqual( denom, 0.0 ) )
		return {};

	auto x = ( b1 * c2 - b2 * c1 ) / denom;
	auto y = ( a2 * c1 - a1 * c2 ) / denom;

	if ( !infinite )
	{
		// coincident points do not count as intersecting
		if ( fabs( a.m_x - b.m_x ) > DBL_TOL &&
			 ( ( a.m_x < b.m_x ) ? x < a.m_x || x > b.m_x : x > a.m_x || x < b.m_x ) )
			return {};
		if ( fabs( a.m_y - b.m_y ) > DBL_TOL &&
			 ( ( a.m_y < b.m_y ) ? y < a.m_y || y > b.m_y : y > a.m_y || y < b.m_y ) )
			return {};

		if ( fabs( e.m_x - f.m_x ) > DBL_TOL &&
			 ( ( e.m_x < f.m_x ) ? x < e.m_x || x > f.m_x : x > e.m_x || x < f.m_x ) )
			return {};
		if ( fabs( e.m_y - f.m_y ) > DBL_TOL &&
			 ( ( e.m_y < f.m_y ) ? y < e.m_y || y > f.m_y : y > e.m_y || y < f.m_y ) )
			return {};
	}

	return { x, y };
}
