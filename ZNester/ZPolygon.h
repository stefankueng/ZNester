#pragma once
#include <deque>
#include <memory>

#include "ZRect.h"

enum class ePointInside
{
	Invalid,
	Inside,
	Outside
};

class ZPolygon : public std::deque<ZPoint>
{
public:
	ZPolygon();
	ZPolygon( size_t id, size_t rotations );
	ZPolygon( const ZPolygon& other );
	ZPolygon( ZPolygon&& other ) noexcept;
	ZPolygon& operator=( const ZPolygon& other );
	ZPolygon& operator=( ZPolygon&& other ) noexcept;
	~ZPolygon() {}

	bool isValid() const;

	// bounds rectangle of the polygon
	ZRect bounds() const;
	// area, can be negative!
	double area() const;
	// true if the polygon is ordered anti clock wise
	bool isAntiClockWise() const;
	// reverses the orientation
	void reverse();

	// translate the polygon
	void	  translate( const double& dx, const double& dy );
	void	  translate( const ZPoint& diffPt );
	ZPolygon& operator-=( const ZPoint& pt );
	ZPolygon& operator+=( const ZPoint& pt );
	ZPolygon& operator*=( double scale );
	// returns a copy of the polygon, translated accordingly
	ZPolygon	 translated( const double& dx, const double& dy ) const;
	ZPolygon	 translated( const ZPoint& diffPt ) const;
	ZPolygon	 scaled( double scale ) const;

	ePointInside isPointInside( const ZPoint& pt, bool useOffset ) const;
	bool		 isRectangle() const;

	// rotate the polygon
	void rotate( double radAngle );
	// returns a rotated copy of the polygon
	ZPolygon rotated( double radAngle ) const;

	// shrinks/enlarges the polygon (enlarge with negative offset)
	void shrink( double offset );

	// returns the union of this polygon with 'other'
	std::deque<ZPolygon> united( const ZPolygon& other ) const;
	// returns the difference of this polygon minus all 'other' polygons
	ZPolygon difference( const std::deque<ZPolygon>& other ) const;

	// returns true if this polygon intersects with poly
	bool intersect( const ZPolygon& poly ) const;
	// returns the distance of this polygon to edgeB in direction 'direction'
	double projectionDistance( const ZPolygon& edgeB, const ZPoint& direction ) const;
	// returns the distance polygon b can slide along this polygon in direction
	double			slideDistance( const ZPolygon& b, const ZPoint& direction, bool ignoreNegative ) const;

	static ZPolygon convexHull( ZPolygon& points );

	// adds a child polygon, usually a hole
	void addChild( const ZPolygon& child ) const;
	// returns the child polygons
	std::deque<ZPolygon>& children() const;

	// get/set the id of the polygon
	__forceinline size_t id() const
	{
		return m_id;
	}
	void setId( size_t id );
	// get/set the rotation of the polygon, and the possible rotations
	__forceinline size_t rotations() const
	{
		return m_rotations;
	}
	__forceinline double rotation() const
	{
		return m_rotation;
	}
	void			   setRotation( const double& r );
	void			   setRotations( size_t rots );

	__forceinline size_t copies() const
	{
		return m_copies;
	}
	__forceinline size_t copy() const
	{
		return m_copy;
	} 
	__forceinline void setCopies( size_t copies )
	{
		m_copies = copies;
	}

	// get/set the offset of the polygon, without actually translating the polygon
	__forceinline double offsetX() const
	{
		return m_offset.x();
	}
	__forceinline double offsetY() const
	{
		return m_offset.y();
	}
	__forceinline const ZPoint& offset() const
	{
		return m_offset;
	}
	void setOffset( const ZPoint& off );
	void setOffset( double off );
	void setOffsetX( double x );
	void setOffsetY( double y );

private:
	size_t								  m_id		  = 0;
	size_t								  m_rotations = 10;
	double								  m_rotation  = 0.0;
	size_t								  m_copies	  = 2;
	size_t								  m_copy	  = 0;
	ZPoint								  m_offset	  = { 0, 0 };
	std::unique_ptr<std::deque<ZPolygon>> m_children;

	friend class ZNester;
};
