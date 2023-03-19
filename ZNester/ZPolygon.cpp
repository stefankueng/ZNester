#include "pch.h"

#include "ZPolygon.h"

#include <clipper2/clipper.h>

ZPolygon::ZPolygon()
{
	m_children = std::make_unique<std::deque<ZPolygon>>();
}

inline ZPolygon::ZPolygon( size_t id, size_t rotations )
	: m_id( id )
	, m_rotations( rotations )
{
	m_children = std::make_unique<std::deque<ZPolygon>>();
}

ZPolygon::ZPolygon( const ZPolygon& other )
	: std::deque<ZPoint>( other )
{
	m_id		= other.m_id;
	m_rotations = other.m_rotations;
	m_rotation	= other.m_rotation;
	m_copies	= other.m_copies;
	m_copy		= other.m_copy;
	m_offset	= other.m_offset;
	m_children	= std::make_unique<std::deque<ZPolygon>>( *other.m_children );
}

ZPolygon::ZPolygon( ZPolygon&& other ) noexcept
	: std::deque<ZPoint>( std::move( other ) )
{
	m_id		= other.m_id;
	m_rotations = other.m_rotations;
	m_rotation	= other.m_rotation;
	m_copies	= other.m_copies;
	m_copy		= other.m_copy;
	m_offset	= std::move( other.m_offset );
	m_children	= std::move( other.m_children );
}

ZPolygon& ZPolygon::operator=( const ZPolygon& other )
{
	__super::operator=( other );
	m_id			 = other.m_id;
	m_rotations		 = other.m_rotations;
	m_rotation		 = other.m_rotation;
	m_copies		 = other.m_copies;
	m_copy			 = other.m_copy;
	m_offset		 = other.m_offset;
	m_children		 = std::make_unique<std::deque<ZPolygon>>( *other.m_children );
	return *this;
}

ZPolygon& ZPolygon::operator=( ZPolygon&& other ) noexcept
{
	__super::operator=( other );
	m_id			 = other.m_id;
	m_rotations		 = other.m_rotations;
	m_rotation		 = other.m_rotation;
	m_copies		 = other.m_copies;
	m_copy			 = other.m_copy;
	m_offset		 = std::move( other.m_offset );
	m_children		 = std::move( other.m_children );
	return *this;
}

ZRect ZPolygon::bounds() const
{
	if ( size() < 3 )
	{
		return {};
	}

	auto xMin = DBL_MAX;
	auto xMax = -DBL_MAX;
	auto yMin = DBL_MAX;
	auto yMax = -DBL_MAX;

	for ( const auto& pt : *this )
	{
		xMin = std::min( xMin, pt.x() );
		yMin = std::min( yMin, pt.y() );
		xMax = std::max( xMax, pt.x() );
		yMax = std::max( yMax, pt.y() );
	}
	return { xMin, yMin, xMax - xMin, yMax - yMin };
}

double ZPolygon::area() const
{
	double area = 0;
	for ( size_t i = 0, j = size() - 1ULL; i < size(); j = i++ )
	{
		area += ( at( j ).x() + at( i ).x() ) * ( at( j ).y() - at( i ).y() );
	}
	return 0.5 * area;
}

bool ZPolygon::isAntiClockWise() const
{
	return area() < 0;
}

void ZPolygon::translate( const double& dx, const double& dy )
{
	const ZPoint diffPt( dx, dy );
	translate( diffPt );
}

void ZPolygon::translate( const ZPoint& diffPt )
{
	for ( auto& pt : *this )
		pt += diffPt;
}

ZPolygon ZPolygon::translated( const double& dx, const double& dy ) const
{
	auto retPoly = *this;
	retPoly.translate( dx, dy );
	return retPoly;
}

ZPolygon ZPolygon::translated( const ZPoint& diffPt ) const
{
	auto retPoly = *this;
	retPoly.translate( diffPt );
	return retPoly;
}

ZPolygon ZPolygon::scaled( double scale ) const
{
	auto retPoly = *this;
	for ( auto& pt : retPoly )
		pt *= scale;
	return retPoly;
}

void ZPolygon::reverse()
{
	std::reverse( begin(), end() );
}

ePointInside ZPolygon::isPointInside( const ZPoint& pt, bool useOffset ) const
{
	if ( empty() )
	{
		return ePointInside::Outside;
	}

	bool inside = false;

	auto off	= useOffset ? offset() : ZPoint( 0.0, 0.0 );
	for ( size_t i = 0, j = size() - 1; i < size(); j = i++ )
	{
		auto pi = at( i ) + off;
		auto pj = at( j ) + off;

		if ( pi == pt )
		{
			return ePointInside::Invalid;  // no result
		}

		if ( pt.onSegment( pi, pj ) )
		{
			return ePointInside::Invalid;  // exactly on the segment
		}

		bool intersect = ( ( pi.y() > pt.y() ) != ( pj.y() > pt.y() ) ) &&
						 ( pt.x() < ( pj.x() - pi.x() ) * ( pt.y() - pi.y() ) / ( pj.y() - pi.y() ) + pi.x() );
		if ( intersect )
			inside = !inside;
	}

	return inside ? ePointInside::Inside : ePointInside::Outside;
}

bool ZPolygon::isRectangle() const
{
	if ( size() != 4 )
		return false;
	auto bounds = this->bounds();
	for ( const auto& pt : *this )
	{
		if ( !dblEqual( pt.x(), bounds.x() ) && !dblEqual( pt.x(), bounds.x() + bounds.width() ) )
			return false;
		if ( !dblEqual( pt.y(), bounds.y() ) && !dblEqual( pt.y(), bounds.y() + bounds.height() ) )
			return false;
	}

	return true;
}

bool ZPolygon::isValid() const
{
	return size() > 2;
}

ZPolygon& ZPolygon::operator-=( const ZPoint& pt )
{
	translate( -pt );
	return *this;
}

ZPolygon& ZPolygon::operator+=( const ZPoint& pt )
{
	translate( pt );
	return *this;
}

ZPolygon& ZPolygon::operator*=( double scale )
{
	for ( auto& pt : *this )
		pt *= scale;
	return *this;
}

void ZPolygon::rotate( double radAngle )
{
	auto sinA = sin( radAngle );
	auto cosA = cos( radAngle );
	for ( auto& pt : *this )
	{
		auto x = pt.x();
		auto y = pt.y();
		pt.setX( x * cosA - y * sinA );
		pt.setY( x * sinA + y * cosA );
	}
	m_rotation = radAngle;
}
ZPolygon ZPolygon::rotated( double radAngle ) const
{
	auto	 sinA = sin( radAngle );
	auto	 cosA = cos( radAngle );
	ZPolygon rot( *this );
	for ( auto& pt : rot )
	{
		auto x	= pt.x();
		auto y	= pt.y();
		auto x1 = x * cosA - y * sinA;
		auto y1 = x * sinA + y * cosA;
		pt		= { x1, y1 };
	}
	rot.m_rotation = radAngle;
	for ( auto& child : *rot.m_children )
	{
		child = child.rotated( radAngle );
	}
	return rot;
}

void ZPolygon::shrink( double offset )
{
	if ( !isValid() )
		return;

	bool				isAntiClockWise = this->isAntiClockWise();

	Clipper2Lib::PathsD paths;
	Clipper2Lib::PathD	path;
	for ( const auto& pt : *this )
		path.emplace_back( pt.x() * CLIPPER_LIB_SCALE, pt.y() * CLIPPER_LIB_SCALE );
	paths.push_back( path );
	auto result = Clipper2Lib::InflatePaths( paths, -offset * CLIPPER_LIB_SCALE, Clipper2Lib::JoinType::Miter,
											 Clipper2Lib::EndType::Polygon );

	if ( result.size() != 1 )
		return;

	clear();

	for ( const auto& pt : result[0] )
	{
		emplace_back( pt.x / CLIPPER_LIB_SCALE, pt.y / CLIPPER_LIB_SCALE );
	}

	if ( isAntiClockWise != this->isAntiClockWise() )
		reverse();
}

std::deque<ZPolygon> ZPolygon::united( const ZPolygon& other ) const
{
	Clipper2Lib::PathsD paths;
	Clipper2Lib::PathD	path;
	Clipper2Lib::PathD	path2;
	for ( const auto& pt : *this )
		path.emplace_back( pt.x() * CLIPPER_LIB_SCALE, pt.y() * CLIPPER_LIB_SCALE );
	paths.push_back( path );
	for ( const auto& pt : other )
		path2.emplace_back( pt.x() * CLIPPER_LIB_SCALE, pt.y() * CLIPPER_LIB_SCALE );
	paths.push_back( path2 );

	auto				 result = Clipper2Lib::Union( paths, Clipper2Lib::FillRule::NonZero );

	std::deque<ZPolygon> unitedPolys;
	for ( const auto& poly : result )
	{
		ZPolygon unitedPoly;
		for ( const auto& pt : poly )
		{
			unitedPoly.emplace_back( pt.x / CLIPPER_LIB_SCALE, pt.y / CLIPPER_LIB_SCALE );
		}
		unitedPolys.push_back( unitedPoly );
		break;
	}

	return unitedPolys;
}

ZPolygon ZPolygon::difference( const std::deque<ZPolygon>& other ) const
{
	Clipper2Lib::PathsD subjects;
	Clipper2Lib::PathsD clips;
	Clipper2Lib::PathD	path;
	for ( const auto& pt : *this )
		path.emplace_back( pt.x() * CLIPPER_LIB_SCALE, pt.y() * CLIPPER_LIB_SCALE );
	subjects.push_back( path );
	for ( const auto& poly : other )
	{
		Clipper2Lib::PathD path2;
		for ( const auto& pt : poly )
			path2.emplace_back( pt.x() * CLIPPER_LIB_SCALE, pt.y() * CLIPPER_LIB_SCALE );
		clips.push_back( path2 );
	}

	auto result = Clipper2Lib::Difference( subjects, clips, Clipper2Lib::FillRule::NonZero );

	if ( result.size() != 1 )
		return {};

	ZPolygon diffPoly;
	for ( const auto& pt : result[0] )
	{
		diffPoly.emplace_back( pt.x / CLIPPER_LIB_SCALE, pt.y / CLIPPER_LIB_SCALE );
	}

	return diffPoly;
}

bool ZPolygon::intersect( const ZPolygon& poly ) const
{
	auto aOffset = offset();
	auto bOffset = poly.offset();

	for ( size_t i = 0; i < size(); ++i )
	{
		auto   prevAIndex  = ( i == 0 ) ? size() - 1 : i - 1;
		size_t nextAIndex  = i == size() - 1 ? 0 : i + 1;
		auto   next2AIndex = ( i + 2 ) % size();

		auto   a0		   = at( prevAIndex ) + aOffset;
		auto   a1		   = at( i ) + aOffset;
		auto   a2		   = at( nextAIndex ) + aOffset;
		auto   a3		   = at( next2AIndex ) + aOffset;

		for ( size_t j = 0; j < poly.size(); ++j )
		{
			auto   prevBIndex  = ( j == 0 ) ? poly.size() - 1 : j - 1;
			size_t nextBIndex  = j == poly.size() - 1 ? 0 : j + 1;
			auto   next2BIndex = ( j + 2 ) % poly.size();

			auto   b0		   = poly[prevBIndex] + bOffset;
			auto   b1		   = poly[j] + bOffset;
			auto   b2		   = poly[nextBIndex] + bOffset;
			auto   b3		   = poly[next2BIndex] + bOffset;

			if ( a1 == b1 || b1.onSegment( a1, a2 ) )
			{
				// if a point is on a segment, it could intersect or it could not.
				// Check via the neighboring points
				auto b0In = isPointInside( b0, true );
				auto b2In = isPointInside( b2, true );
				if ( ( b0In == ePointInside::Inside && b2In == ePointInside::Outside ) ||
					 ( b0In == ePointInside::Outside && b2In == ePointInside::Inside ) )
				{
					return true;
				}
				else
				{
					auto b3In = isPointInside( b3, true );
					if ( ( b0In == ePointInside::Inside && b3In == ePointInside::Outside ) ||
						 ( b0In == ePointInside::Outside && b3In == ePointInside::Inside ) )
					{
						return true;
					}
					continue;
				}
			}

			if ( a2 == b2 || b2.onSegment( a1, a2 ) )
			{
				// if a point is on a segment, it could intersect or it could not.
				// Check via the neighboring points
				auto b1In = isPointInside( b1, true );
				auto b3In = isPointInside( b3, true );

				if ( ( b1In == ePointInside::Inside && b3In == ePointInside::Outside ) ||
					 ( b1In == ePointInside::Outside && b3In == ePointInside::Inside ) )
				{
					return true;
				}
				else
				{
					auto b0In = isPointInside( b0, true );
					if ( ( b0In == ePointInside::Inside && b3In == ePointInside::Outside ) ||
						 ( b0In == ePointInside::Outside && b3In == ePointInside::Inside ) )
					{
						return true;
					}
					continue;
				}
			}

			if ( a1 == b2 || a1.onSegment( b1, b2 ) )
			{
				// if a point is on a segment, it could intersect or it could not.
				// Check via the neighboring points
				auto a0In = poly.isPointInside( a0, true );
				auto a2In = poly.isPointInside( a2, true );

				if ( ( a0In == ePointInside::Inside && a2In == ePointInside::Outside ) ||
					 ( a0In == ePointInside::Outside && a2In == ePointInside::Inside ) )
				{
					return true;
				}
				else
				{
					auto a3In = isPointInside( a3, true );
					if ( ( a0In == ePointInside::Inside && a3In == ePointInside::Outside ) ||
						 ( a0In == ePointInside::Outside && a3In == ePointInside::Inside ) )
					{
						return true;
					}
					continue;
				}
			}

			if ( a2 == b1 || a2.onSegment( b1, b2 ) )
			{
				// if a point is on a segment, it could intersect or it could not.
				// Check via the neighboring points
				auto a1In = poly.isPointInside( a1, true );
				auto a3In = poly.isPointInside( a3, true );

				if ( ( a1In == ePointInside::Inside && a3In == ePointInside::Outside ) ||
					 ( a1In == ePointInside::Outside && a3In == ePointInside::Inside ) )
				{
					return true;
				}
				else
				{
					auto a0In = isPointInside( a0, true );
					if ( ( a0In == ePointInside::Inside && a3In == ePointInside::Outside ) ||
						 ( a0In == ePointInside::Outside && a3In == ePointInside::Inside ) )
					{
						return true;
					}
					continue;
				}
			}

			auto p = ZPoint::lineIntersect( b1, b2, a1, a2 );

			if ( p.isValid() )
			{
				return true;
			}
		}
	}

	return false;
}

double ZPolygon::projectionDistance( const ZPolygon& edgeB, const ZPoint& direction ) const
{
	double distance = DBL_MAX;

	for ( const auto& edge : edgeB )
	{
		// the shortest/most negative projection of B onto A
		ZPoint p			 = edge + edgeB.offset();
		double minProjection = DBL_MAX;
		for ( int j = 0; j < size(); ++j )
		{
			ZPoint s1 = at( j ) + offset();
			ZPoint s2 = at( ( j + 1ULL ) == size() ? 0 : j + 1ULL ) + offset();

			if ( abs( ( s2 - s1 ).cross( direction ) ) < DBL_TOL )
				continue;

			// project point, ignore edge boundaries
			double d = ZPoint::pointDistance( p, s1, s2, direction );

			if ( d != DBL_MAX && ( minProjection == DBL_MAX || d < minProjection ) )
			{
				minProjection = d;
			}
		}
		if ( minProjection != DBL_MAX && ( distance == DBL_MAX || minProjection > distance ) )
		{
			distance = minProjection;
		}
	}

	return distance;
}

double ZPolygon::slideDistance( const ZPolygon& b, const ZPoint& direction, bool ignoreNegative ) const
{
	double distance = DBL_MAX;
	ZPoint dir		= direction.normalized();

	for ( int i = 0; i < b.size(); ++i )
	{
		auto offB = ( ( i == ( b.size() - 1 ) ) ? 0 : i + 1 );
		auto b1	  = b[i] + b.offset();
		auto b2	  = b[offB] + b.offset();
		for ( int j = 0; j < size(); ++j )
		{
			auto offA = ( ( j == ( size() - 1 ) ) ? 0 : j + 1 );
			auto a1	  = at( j ) + offset();
			auto a2	  = at( offA ) + offset();

			auto d	  = ZPoint::segmentDistance( a1, a2, b1, b2, dir );

			if ( ( d < distance ) && ( !ignoreNegative || d > 0 || dblEqual( d, 0.0 ) ) )
			{
				distance = d;
			}
			if ( ignoreNegative && dblEqual( distance, 0.0 ) )
				return distance;
		}
	}
	return distance;
}

__forceinline double ccw( const ZPoint& a, const ZPoint& b, const ZPoint& c )
{
	return ( b.x() - a.x() ) * ( c.y() - a.y() ) - ( b.y() - a.y() ) * ( c.x() - a.x() );
}
__forceinline bool isLeftOf( const ZPoint& a, const ZPoint& b )
{
	return a < b;
}

// Used to sort points in ccw order about a pivot.
struct CcwSorter
{
	const ZPoint& pivot;

	explicit CcwSorter( const ZPoint& inPivot )
		: pivot( inPivot )
	{
	}

	bool operator()( const ZPoint& a, const ZPoint& b ) const
	{
		return ccw( pivot, a, b ) < 0;
	}
};

ZPolygon ZPolygon::convexHull( ZPolygon& points )
{
	// Put our leftmost point at index 0
	std::swap( points[0], *std::ranges::min_element( points, isLeftOf ) );

	// Sort the rest of the points in counter-clockwise order
	// from our leftmost point.
	sort( points.begin() + 1, points.end(), CcwSorter( points[0] ) );

	// Add our first three points to the hull.
	ZPolygon hull;
	auto	 it = points.begin();
	hull.push_back( *it++ );
	hull.push_back( *it++ );
	hull.push_back( *it++ );

	size_t top = 2;
	while ( it != points.end() )
	{
		// Pop off any points that make a convex angle with *it
		while ( top > 0 && ccw( *( hull.rbegin() + 1 ), *( hull.rbegin() ), *it ) >= 0 )
		{
			--top;
			hull.pop_back();
		}
		hull.push_back( *it++ );
		++top;
	}

	return hull;
}

void ZPolygon::addChild( const ZPolygon& child ) const
{
	m_children->push_back( child );
}

std::deque<ZPolygon>& ZPolygon::children() const
{
	return *m_children;
}

void ZPolygon::setId( size_t id )
{
	m_id = id;
}
void ZPolygon::setRotation( const double& r )
{
	m_rotation = r;
}

void ZPolygon::setRotations( size_t rots )
{
	m_rotations = rots;
}

void ZPolygon::setOffset( const ZPoint& off )
{
	m_offset = off;
	for ( auto& child : *m_children )
		child.m_offset = off;
}

void ZPolygon::setOffset( double off )
{
	setOffset( { off, off } );
}

void ZPolygon::setOffsetX( double x )
{
	m_offset.setX( x );
	for ( auto& child : *m_children )
		child.m_offset.setX( x );
}

void ZPolygon::setOffsetY( double y )
{
	m_offset.setY( y );
	for ( auto& child : *m_children )
		child.m_offset.setY( y );
}
