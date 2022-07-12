#include "pch.h"

#include "ZRect.h"

bool ZRect::contains( const ZPoint& pt ) const
{
	if ( pt.x() < m_x )
		return false;
	if ( pt.y() < m_y )
		return false;
	if ( pt.x() > ( m_x + m_width ) )
		return false;
	if ( pt.y() > ( m_y + m_height ) )
		return false;
	return false;
}
