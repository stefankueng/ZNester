#pragma once
#include <deque>

#include "ZGeomUtil.h"
constexpr double DBL_ANG_TOL = 1e-7;

class AngleRange
{
public:
	explicit AngleRange( double start, double end )
		: m_start( start )
		, m_end( end )
	{
	}

	bool angleInRange( double angle ) const
	{
		if ( m_start < m_end )
		{
			if ( dblSmaller( angle, m_start, DBL_ANG_TOL ) || dblLarger( angle, m_end, DBL_ANG_TOL ) )
			{
				return false;
			}
		}
		else
		{
			if ( dblSmaller( angle, m_start, DBL_ANG_TOL ) && dblLarger( angle, m_end, DBL_ANG_TOL ) )
			{
				return false;
			}
		}
		return true;
	}

private:
	double m_start;
	double m_end;
};

class AngleRanges
{
public:
	AngleRanges()  = default;
	~AngleRanges() = default;

	void addRange( double start, double end )
	{
		m_ranges.emplace_back( start, end );
	}

	bool angleInRange( double angle ) const
	{
		for ( const auto& range : m_ranges )
		{
			if ( !range.angleInRange( angle ) )
				return false;
		}
		return true;
	}

private:
	std::deque<AngleRange> m_ranges;
};