#pragma once
#include "ZPoint.h"

class ZRect
{
public:
	ZRect() = default;

	ZRect( const double& x, const double& y, const double& width, const double& height )
		: m_x( x )
		, m_y( y )
		, m_width( width )
		, m_height( height )
	{
	}

	__forceinline double x() const
	{
		return m_x;
	}

	__forceinline double y() const
	{
		return m_y;
	}

	__forceinline double width() const
	{
		return m_width;
	}

	__forceinline double height() const
	{
		return m_height;
	}

	__forceinline double area() const
	{
		return m_width * m_height;
	}

	bool contains( const ZPoint& pt ) const;

private:
	double m_x      = DBL_MAX;
	double m_y      = DBL_MAX;
	double m_width  = DBL_MAX;
	double m_height = DBL_MAX;
};
