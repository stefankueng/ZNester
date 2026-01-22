#include "pch.h"

#include <corecrt_math_defines.h>

#include "../ZNester/ZPoint.h"
#include "../ZNester/ZPolygon.h"

TEST( TestZPoint, ZPoint )
{
	ZPoint a( 1.0, 1.0 );
	ZPoint b( 3.0, 3.0 );
	auto   c = a - b;
	ASSERT_DOUBLE_EQ( c.x(), -2.0 );
	ASSERT_DOUBLE_EQ( c.y(), -2.0 );
	ASSERT_DOUBLE_EQ( a.length(), 1.4142135623730951 );
	ASSERT_DOUBLE_EQ( c.normalized().length(), 1.0 );
	ASSERT_DOUBLE_EQ( c.normal().x(), -2.0 );
	ASSERT_DOUBLE_EQ( c.normal().y(), 2.0 );
	ASSERT_TRUE( a.dot( b ) == b.dot( a ) );
	ASSERT_DOUBLE_EQ( a.distance( b ), b.distance( a ) );
	ASSERT_DOUBLE_EQ( a.distance( b ), 2.8284271247461903 );

	ASSERT_TRUE( ZPoint( 2.0, 1.0 ).onSegment( ZPoint( 1.0, 1.0 ), ZPoint( 4.0, 1.0 ) ) );
	ASSERT_FALSE( ZPoint( 2.0, 1.5 ).onSegment( ZPoint( 1.0, 1.0 ), ZPoint( 4.0, 1.0 ) ) );
	ASSERT_FALSE( ZPoint( 4.1, 1.0 ).onSegment( ZPoint( 1.0, 1.0 ), ZPoint( 4.0, 1.0 ) ) );
	ASSERT_FALSE( ZPoint( 0.9, 1.0 ).onSegment( ZPoint( 1.0, 1.0 ), ZPoint( 4.0, 1.0 ) ) );

	ASSERT_TRUE( ZPoint::lineIntersect( ZPoint( 1.0, 1.0 ), ZPoint( 4.0, 1.0 ), ZPoint( 2.0, 2.0 ), ZPoint( 2.0, 0.0 ) )
					 .isValid() );
	ASSERT_TRUE(
		ZPoint::lineIntersect( ZPoint( 1.0, 1.0 ), ZPoint( 4.0, 1.0 ), ZPoint( 2.0, 2.0 ), ZPoint( 2.0, 1.5 ), true )
			.isValid() );
	ASSERT_FALSE(
		ZPoint::lineIntersect( ZPoint( 1.0, 1.0 ), ZPoint( 4.0, 1.0 ), ZPoint( 2.0, 2.0 ), ZPoint( 2.0, 1.5 ) )
			.isValid() );
}

TEST( TestZPolygon, Bounds )
{
	ZPolygon poly;
	poly.push_back( { 1.0, 1.0 } );
	poly.push_back( { 2.0, 1.0 } );
	poly.push_back( { 2.0, 2.0 } );
	poly.push_back( { 1.0, 2.0 } );

	ASSERT_DOUBLE_EQ( poly.bounds().x(), 1.0 );
	ASSERT_DOUBLE_EQ( poly.bounds().y(), 1.0 );
	ASSERT_DOUBLE_EQ( poly.bounds().width(), 1.0 );
	ASSERT_DOUBLE_EQ( poly.bounds().height(), 1.0 );
}

TEST( TestZPolygon, Area )
{
	ZPolygon poly;
	poly.push_back( { 1.0, 1.0 } );
	poly.push_back( { 2.0, 1.0 } );
	poly.push_back( { 2.0, 2.0 } );
	poly.push_back( { 1.0, 2.0 } );

	ASSERT_DOUBLE_EQ( poly.area(), -1.0 );
}

TEST( TestZPolygon, AntiClockWise )
{
	ZPolygon poly;
	poly.push_back( { 1.0, 1.0 } );
	poly.push_back( { 2.0, 1.0 } );
	poly.push_back( { 2.0, 2.0 } );
	poly.push_back( { 1.0, 2.0 } );

	ASSERT_TRUE( poly.isAntiClockWise() );
	poly.reverse();
	ASSERT_FALSE( poly.isAntiClockWise() );
}

TEST( TestZPolygon, Translate )
{
	ZPolygon poly;
	poly.push_back( { 1.0, 1.0 } );
	poly.push_back( { 2.0, 1.0 } );
	poly.push_back( { 2.0, 2.0 } );
	poly.push_back( { 1.0, 2.0 } );

	auto poly2 = poly;
	poly.translate( 1.0, 1.0 );

	ASSERT_DOUBLE_EQ( poly[0].x(), 2.0 );
	ASSERT_DOUBLE_EQ( poly[0].y(), 2.0 );
	ASSERT_DOUBLE_EQ( poly[1].x(), 3.0 );
	ASSERT_DOUBLE_EQ( poly[1].y(), 2.0 );
	ASSERT_DOUBLE_EQ( poly[2].x(), 3.0 );
	ASSERT_DOUBLE_EQ( poly[2].y(), 3.0 );
	ASSERT_DOUBLE_EQ( poly[3].x(), 2.0 );
	ASSERT_DOUBLE_EQ( poly[3].y(), 3.0 );

	for ( size_t i = 0; i < poly.size(); ++i )
		ASSERT_TRUE( poly.translated( -1.0, -1.0 )[i] == poly2[i] );
}

TEST( TestZPolygon, PointInside )
{
	ZPolygon poly;
	poly.push_back( { 1.0, 1.0 } );
	poly.push_back( { 2.0, 1.0 } );
	poly.push_back( { 2.0, 2.0 } );
	poly.push_back( { 1.0, 2.0 } );
	poly.setOffset( { 2.0, 3.0 } );

	ASSERT_TRUE( poly.isPointInside( { 1.5, 1.5 }, false ) == ePointInside::Inside );
	ASSERT_TRUE( poly.isPointInside( { 1.5, 1.5 }, true ) == ePointInside::Outside );
	ASSERT_TRUE( poly.isPointInside( { 1.0, 1.0 }, false ) == ePointInside::Invalid );
	ASSERT_TRUE( poly.isPointInside( { 3.5, 4.5 }, true ) == ePointInside::Inside );
}

TEST( TestZPolygon, Rotate )
{
	ZPolygon poly;
	poly.push_back( { 1.0, 1.0 } );
	poly.push_back( { 3.0, 1.0 } );
	poly.push_back( { 3.0, 2.0 } );
	poly.push_back( { 1.0, 2.0 } );

	auto poly2 = poly.rotated( M_PI_2 );

	ASSERT_DOUBLE_EQ( poly2[0].x(), -1.0 );
	ASSERT_DOUBLE_EQ( poly2[0].y(), 1.0 );
	ASSERT_DOUBLE_EQ( poly2[1].x(), -1.0 );
	ASSERT_DOUBLE_EQ( poly2[1].y(), 3.0 );
	ASSERT_DOUBLE_EQ( poly2[2].x(), -2.0 );
	ASSERT_DOUBLE_EQ( poly2[2].y(), 3.0 );
	ASSERT_DOUBLE_EQ( poly2[3].x(), -2.0 );
	ASSERT_DOUBLE_EQ( poly2[3].y(), 1.0 );
}
