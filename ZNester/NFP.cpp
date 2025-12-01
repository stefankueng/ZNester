#include "pch.h"

#include "NFP.h"

#include <clipper2/clipper.h>
#include <intsafe.h>

#include <algorithm>
#include <format>

#include "AngleRanges.h"

struct TransVector
{
	ZPoint	pt;
	ZPoint *start	= nullptr;
	ZPoint *end		= nullptr;
	double	dMax	= 0.0;
	double	dMax2	= 0.0;
	double	dot		= 0.0;
	double	length2 = 0.0;
	double	scale	= 1.0;
	double	length	= 0.0;	// only set if scale is != 1.0
	bool	inNfp	= false;
	bool	back	= false;
};

struct EdgeDescriptor
{
	int	   type	 = 0;
	size_t edgeA = 0;
	size_t edgeB = 0;
};

bool inNfp( const ZPoint &p, const std::deque<ZPolygon> &nfp )
{
	if ( nfp.empty() )
	{
		return false;
	}
	for ( const auto &poly : nfp )
	{
		for ( const auto &pt : poly )
		{
			if ( p == pt )
			{
				return true;
			}
		}
	}

	return false;
}

enum eAlignment
{
	Left,
	Right,
	On
};

void minkowskiFallback( std::deque<ZPolygon>											&nfpList,
						const std::function<void( eZLogLevel, const std::string &msg )> &logCallback, const ZPolygon &a,
						const ZPolygon &b, bool inside, const tDebugCallback &debugDisplay, ZPolygon &nfp )
{
	if ( nfpList.empty() )
	{
		// didn't close the loop, something went wrong here
		// fall back using the Minkowsky difference
		if ( logCallback )
			logCallback( Debug, std::format( "failed to generate nfp with orbiting approach for {0}, {1} "
											 "{2}, using minkowski difference instead.",
											 a.id(), b.id(), inside ? "inside" : "outside" ) );
		nfpList = noFitPolygonMinkowski( a, b, inside, logCallback, debugDisplay );
	}
	nfp.clear();
}

eAlignment getAlignment( const ZPoint &segStart, const ZPoint &segEnd, const ZPoint &pt )
{
	auto res = ( ( segEnd.x() - segStart.x() ) * ( pt.y() - segStart.y() ) -
				 ( segEnd.y() - segStart.y() ) * ( pt.x() - segStart.x() ) );

	if ( dblEqual( res, 0 ) )
	{
		return On;
	}
	else if ( res > 0 )
	{
		return Left;
	}
	else
	{
		return Right;
	}
}

ZPoint searchStartPoint( ZPolygon &a, ZPolygon &b, bool inside, const std::deque<ZPolygon> &nfp,
						 [[maybe_unused]] const std::function<void( eZLogLevel, const std::string &msg )> &logCallback,
						 [[maybe_unused]] const tDebugCallback &debugDisplay )
{
	for ( int i = 0; i < a.size(); ++i )
	{
		if ( !a[i].marked() )
		{
			a[i].setMarked( true );
			for ( int j = 0; j < b.size(); ++j )
			{
				b.setOffset( a[i] - b[j] );

				// if (debugDisplay)
				//{
				//     ZPolygon aCopy;
				//     ZPolygon bCopy;
				//     for (const auto &pt : a)
				//         aCopy.emplace_back(pt.x() + a.offsetX(), pt.y() + a.offsetY());
				//     for (const auto &pt : b)
				//         bCopy.emplace_back(pt.x() + b.offsetX(), pt.y() + b.offsetY());
				//     debugDisplay({aCopy, bCopy});
				// }

				ePointInside bInside = ePointInside::Invalid;
				for ( const auto &bPt : b )
				{
					auto inPoly = a.isPointInside( bPt + b.offset(), true );
					if ( inPoly != ePointInside::Invalid )
					{
						bInside = inPoly;
						break;
					}
				}

				if ( bInside == ePointInside::Invalid )
				{
					// A and B are the same
					return {};
				}

				ZPoint startPoint = b.offset();
				if ( ( ( bInside == ePointInside::Inside && inside ) ||
					   ( bInside == ePointInside::Outside && !inside ) ) &&
					 !a.intersect( b ) && !inNfp( startPoint, nfp ) )
				{
					return startPoint;
				}

				// slide B along vector
				auto   next = ( i + 1ULL ) == a.size() ? 0 : i + 1;
				auto   v	= a[next] - a[i];

				double d1	= a.projectionDistance( b, v );
				double d2	= b.projectionDistance( a, -v );

				double d	= DBL_MAX;

				if ( d1 != DBL_MAX || d2 != DBL_MAX )
					d = std::min( d1, d2 );

				// only slide until no longer negative
				if ( d == DBL_MAX || dblEqual( d, 0 ) || d <= 0 )
					continue;

				double vd2 = v.length2();

				if ( d * d < vd2 && !dblEqual( d * d, vd2 ) )
				{
					double vd = v.length();
					v *= d / vd;
				}

				b.setOffset( b.offset() + v );

				// if (debugDisplay)
				//{
				//     ZPolygon aCopy;
				//     ZPolygon bCopy;
				//     for (const auto &pt : a)
				//         aCopy.emplace_back(pt.x() + a.offsetX(), pt.y() + a.offsetY());
				//     for (const auto &pt : b)
				//         bCopy.emplace_back(pt.x() + b.offsetX(), pt.y() + b.offsetY());
				//     debugDisplay({aCopy, bCopy});
				// }

				for ( const auto &bPt : b )
				{
					auto inPoly = a.isPointInside( bPt + b.offset(), true );
					if ( inPoly != ePointInside::Invalid )
					{
						bInside = inPoly;
						break;
					}
				}
				startPoint = b.offset();
				if ( ( ( bInside == ePointInside::Inside && inside ) ||
					   ( bInside == ePointInside::Outside && !inside ) ) &&
					 !a.intersect( b ) && !inNfp( startPoint, nfp ) )
				{
					return startPoint;
				}
			}
		}
	}

	return {};
}

std::deque<ZPolygon> noFitPolygonRectangle( const ZPolygon &a, const ZPolygon &b )
{
	auto boundsA = a.bounds();
	auto boundsB = b.bounds();

	if ( boundsB.width() > boundsA.width() )
	{
		return {};
	}
	if ( boundsB.height() > boundsA.height() )
	{
		return {};
	}
	ZPolygon	poly;
	const auto &b0 = b[0];
	poly.emplace_back( boundsA.x() - boundsB.x() + b0.x(), boundsA.y() - boundsB.y() + b0.y() );
	poly.emplace_back( boundsA.x() + boundsA.width() - ( boundsB.x() + boundsB.width() ) + b0.x(),
					   boundsA.y() - boundsB.y() + b0.y() );
	poly.emplace_back( boundsA.x() + boundsA.width() - ( boundsB.x() + boundsB.width() ) + b0.x(),
					   boundsA.y() + boundsA.height() - ( boundsB.y() + boundsB.height() ) + b0.y() );
	poly.emplace_back( boundsA.x() - boundsB.x() + b0.x(),
					   boundsA.y() + boundsA.height() - ( boundsB.y() + boundsB.height() ) + b0.y() );

	return { poly };
}

std::deque<ZPolygon> noFitPolygon( ZPolygon &a, ZPolygon &b, bool inside, bool searchEdges,
								   const std::function<void( eZLogLevel, const std::string &msg )> &logCallback,
								   const tDebugCallback											   &debugDisplay )
{
	if ( a.size() < 3 || b.size() < 3 )
	{
		return {};
	}
	a.setOffset( 0.0 );

	auto   minA		 = DBL_MAX;
	size_t minAIndex = 0;
	auto   maxB		 = -DBL_MAX;
	size_t maxBIndex = 0;

	size_t h		 = 0;
	for ( auto &pt : a )
	{
		pt.setMarked( false );
		if ( pt.y() < minA )
		{
			minA	  = pt.y();
			minAIndex = h;
		}
		++h;
	}

	h = 0;
	for ( auto &pt : b )
	{
		pt.setMarked( false );
		if ( pt.y() > maxB )
		{
			maxB	  = pt.y();
			maxBIndex = h;
		}
		++h;
	}

	ZPoint startPoint;
	if ( !inside )
	{
		// shift B such that the bottom-most point of B is at the top-most point of A.
		// This guarantees an initial placement with no intersections
		startPoint = a[minAIndex] - b[maxBIndex];
	}
	else
	{
		// no reliable heuristic for inside
		startPoint = searchStartPoint( a, b, inside, {}, logCallback, debugDisplay );
	}

	// if (debugDisplay)
	//{
	//     ZPolygon aCopy;
	//     ZPolygon bCopy;
	//     for (const auto &pt : a)
	//         aCopy.emplace_back(pt.x() + a.offsetX(), pt.y() + a.offsetY());
	//     for (const auto &pt : b)
	//         bCopy.emplace_back(pt.x() + b.offsetX(), pt.y() + b.offsetY());
	//     debugDisplay({aCopy, bCopy});
	// }

	std::deque<ZPolygon> nfpList;
	while ( startPoint.isValid() )
	{
		b.setOffset( startPoint );

		// maintain a list of touching points/edges
		ZPoint	 prevVector;  // keep track of previous vector to detect backward sliding
		ZPolygon nfp;
		auto	 reference = b[0] + b.offset();
		auto	 start	   = reference;

		nfp.push_back( reference );

		size_t counter		= 0;
		size_t maxCount		= 10 * ( a.size() + b.size() );
		int	   inNfpCounter = 0;
		while ( counter < maxCount )  // sanity check, prevent infinite loop
		{
			std::vector<EdgeDescriptor> touching;
			touching.reserve( 50 );
			// find touching vertices/edges
			for ( size_t i = 0; i < a.size(); ++i )
			{
				for ( size_t j = 0; j < b.size(); ++j )
				{
					auto bWithOffset = b[j] + b.offset();
					if ( a[i] == bWithOffset )
					{
						touching.emplace_back( 0, i, j );
					}
					else if ( auto nextI = ( i == a.size() - 1 ) ? 0 : i + 1; bWithOffset.onSegment( a[i], a[nextI] ) )
					{
						touching.emplace_back( 1, nextI, j );
					}
					else if ( auto nextJ = ( j == b.size() - 1 ) ? 0 : j + 1;
							  a[i].onSegment( bWithOffset, b[nextJ] + b.offset() ) )
					{
						touching.emplace_back( 2, i, nextJ );
					}
				}
			}

			// generate translation vectors from touching vertices/edges
			std::vector<TransVector> vectors;
			vectors.reserve( touching.size() * 4 );
			AngleRanges angleRanges;
			for ( const auto &[type, edgeA, edgeB] : touching )
			{
				auto &vertexA = a[edgeA];
				vertexA.setMarked( true );

				// adjacent A vertices

				auto  prevAIndex = ( edgeA == 0 ) ? a.size() - 1 : edgeA - 1ULL;  // loop
				auto  nextAIndex = ( edgeA == a.size() - 1 ) ? 0 : edgeA + 1ULL;  // loop

				auto &prevA		 = a[prevAIndex];
				auto &nextA		 = a[nextAIndex];

				// adjacent B vertices
				auto &vertexB	 = b[edgeB];

				auto  prevBIndex = ( edgeB == 0 ) ? b.size() - 1 : edgeB - 1ULL;  // loop
				auto  nextBIndex = ( edgeB == b.size() - 1 ) ? 0 : edgeB + 1ULL;  // loop

				auto &prevB		 = b[prevBIndex];
				auto &nextB		 = b[nextBIndex];

				switch ( type )
				{
					case 0:
					{
						TransVector vA1 = { nextA - vertexA, &vertexA, &nextA };
						TransVector vA2 = { prevA - vertexA, &vertexA, &prevA };

						// B vectors need to be inverted
						TransVector vB1 = { vertexB - nextB, &nextB, &vertexB };
						TransVector vB2 = { vertexB - prevB, &prevB, &vertexB };

						angleRanges.addRange( vA1.pt.angle(), vA2.pt.angle() );
						angleRanges.addRange( vB1.pt.angle(), vB2.pt.angle() );

						vectors.push_back( std::move( vA1 ) );
						vectors.push_back( std::move( vA2 ) );
						vectors.push_back( std::move( vB1 ) );
						vectors.push_back( std::move( vB2 ) );
					}
					break;
					case 1:
					{
						// B is on segment APrev-A
						vectors.emplace_back( vertexA - ( vertexB + b.offset() ), &prevA, &vertexA );
						auto s = vectors.back().pt.angle();
						vectors.emplace_back( prevA - ( vertexB + b.offset() ), &vertexA, &prevA );
						auto e = vectors.back().pt.angle();
						angleRanges.addRange( s, e );
					}
					break;
					case 2:
					{
						// A is on segment BPrev-B
						vectors.emplace_back( vertexA - ( vertexB + b.offset() ), &prevB, &vertexB );
						auto s = vectors.back().pt.angle();
						vectors.emplace_back( vertexA - ( prevB + b.offset() ), &vertexB, &prevB );
						auto e = vectors.back().pt.angle();
						angleRanges.addRange( s, e );
					}
					break;
				}
			}
			static bool showDebug = false;

			if ( debugDisplay && showDebug )
			{
				ZPolygon aCopy;
				ZPolygon bCopy;
				for ( const auto &pt : a )
					aCopy.emplace_back( pt.x() + a.offsetX(), pt.y() + a.offsetY() );
				for ( const auto &pt : b )
					bCopy.emplace_back( pt.x() + b.offsetX(), pt.y() + b.offsetY() );
				std::deque<ZPoint> markers;
				for ( const auto &t : touching )
				{
					markers.push_back( a[t.edgeA] );
					markers.push_back( b[t.edgeB] + b.offset() );
				}
				debugDisplay( { aCopy, bCopy, nfp }, markers );
			}

			std::vector<TransVector> feasibleVectors;
			feasibleVectors.reserve( vectors.size() );
			for ( auto &transVec : vectors )
			{
				// reject vectors that would lead to an immediate intersection
				if ( !angleRanges.angleInRange( transVec.pt.angle() ) )
					continue;

				double d = a.slideDistance( b, transVec.pt, true );
				if ( dblEqual( d, 0.0 ) )
				{
					// a zero distance would mean that sliding in this direction
					// leads to an intersection.
					continue;
				}
				transVec.length2 = transVec.pt.length2();
				if ( d == DBL_MAX || d * d > transVec.length2 )
				{
					d			   = sqrt( transVec.length2 );
					transVec.dMax2 = transVec.length2;
				}
				else
					transVec.dMax2 = d * d;
				transVec.dMax = d;
				if ( dblEqual( transVec.dMax, 0.0 ) )
					continue;

				if ( prevVector.isValid() )
				{
					transVec.dot = prevVector.dot( transVec.pt );
					transVec.dot = transVec.dot > -DBL_TOL ? 1.0 : 0.0;
					if ( transVec.pt.dot( prevVector ) < 0 )
					{
						// compare magnitude with unit vectors
						ZPoint unitv = transVec.pt.normalized();

						// we need to scale down to unit vectors to normalize vector length.
						// Could also just do a tan here
						if ( dblEqual( unitv.cross( prevVector ), 0.0 ) )
						{
							transVec.back = true;
						}
					}
				}

				if ( !transVec.back && dblSmaller( transVec.dMax2, transVec.length2 ) )
				{
					transVec.scale	= transVec.dMax;
					transVec.length = sqrt( transVec.length2 );
					transVec.inNfp	= nfp.isPointInside( reference + ( transVec.pt * transVec.scale / transVec.length ),
														 false ) == ePointInside::Invalid;
				}
				else if ( !transVec.back )
					transVec.inNfp = nfp.isPointInside( reference + transVec.pt, false ) == ePointInside::Invalid;
				feasibleVectors.push_back( transVec );
			}

			// sort the vectors so that the longest one is used, unless a shorter
			// vector is available that:
			// - has a smaller change in angle than the previous vector
			// - does not point inside the nfp
			// - does not point directly backwards
			std::ranges::sort( feasibleVectors,
							   [&]( const auto &l, const auto &r )
							   {
								   if ( l.back != r.back )
									   return !l.back;
								   if ( l.inNfp != r.inNfp )
									   return !l.inNfp;
								   if ( l.dot != r.dot )
									   return l.dot > r.dot;
								   if ( l.scale != r.scale )
									   return l.scale > r.scale;
								   return l.dMax > r.dMax;
							   } );

			if ( feasibleVectors.empty() )
			{
				minkowskiFallback( nfpList, logCallback, a, b, inside, debugDisplay, nfp );
				break;
			}

			auto &translate = feasibleVectors[0];
			if ( translate.inNfp )
				++inNfpCounter;
			else
				inNfpCounter = 0;

			translate.start->setMarked( true );
			translate.end->setMarked( true );

			prevVector = translate.pt.normalized();

			// trim
			if ( !dblEqual( translate.scale, 1.0 ) )
			{
				translate.pt *= translate.scale;
				translate.pt /= translate.length;
			}

			reference += translate.pt;

			// this check is important to reject the current nfp in case we go backwards again instead
			// of properly orbiting around polygon A.
			// Unfortunately this can happen due to double floating point calculations...
			if ( feasibleVectors.empty() ||
				 ( translate.back && feasibleVectors.size() > 1 && feasibleVectors[1].back ) ||
				 ( inNfpCounter > std::max( 2, static_cast<int>( nfp.size() ) / 4 ) ) ||
				 ( inNfpCounter * 4 >= nfp.size() ) )
			{
				minkowskiFallback( nfpList, logCallback, a, b, inside, debugDisplay, nfp );
				break;
			}

			if ( reference == start )
			{
				// we've made a full loop
				break;
			}

			// if A and B start on a touching horizontal line, the end point may not be the start point
			bool looped	  = false;
			int	 numCheck = std::min( std::max( 3, static_cast<int>( nfp.size() ) / 8 ), 6 );
			if ( nfp.size() > 8 )
			{
				for ( const auto &pt : nfp )
				{
					if ( reference == pt )
					{
						looped = true;
						break;
					}
					if ( --numCheck <= 0 )
						break;
				}
			}

			if ( looped )
			{
				// we've made a full loop
				break;
			}

			nfp.push_back( reference );

			b.setOffset( reference - b[0] );

			counter++;
		}

		if ( nfpList.empty() && ( ( counter >= maxCount ) || ( nfp.size() < ( std::max( 3ULL, a.size() / 3 ) ) ) ) )
		{
			minkowskiFallback( nfpList, logCallback, a, b, inside, debugDisplay, nfp );
		}

		if ( !nfp.empty() )
		{
			nfpList.push_back( nfp );
		}

		if ( !searchEdges )
		{
			// only get outer NFP or first inner NFP
			break;
		}

		startPoint = searchStartPoint( a, b, inside, nfpList, logCallback, debugDisplay );
	}

	return nfpList;
}

// this is the fallback way of getting an nfp, using the Minkowski difference
// but this is much slower than the orbiting approach above, so this is only
// used if necessary as a fallback.
std::deque<ZPolygon> noFitPolygonMinkowski(
	const ZPolygon &a, const ZPolygon &b, bool inside,
	[[maybe_unused]] const std::function<void( eZLogLevel, const std::string &msg )> &logCallback,
	[[maybe_unused]] const tDebugCallback											 &debugDisplay )
{
	std::deque<ZPolygon> nfpList;
	Clipper2Lib::PathD	 aClipper;
	for ( const auto &pt : a )
	{
		aClipper.emplace_back( pt.x(), pt.y() );
	}
	Clipper2Lib::PathD bClipper;
	for ( const auto &pt : b )
	{
		bClipper.emplace_back( -pt.x(), -pt.y() );
	}

	auto  clipperNfp = Clipper2Lib::MinkowskiSum( aClipper, bClipper, true, 8 );
	auto &b0		 = b[0];
	for ( const auto &poly : clipperNfp )
	{
		ZPolygon nfp;
		for ( const auto &pt : poly )
			nfp.emplace_back( pt.x + b0.x(), pt.y + b0.y() );
		if ( !nfp.empty() )
			nfpList.push_back( nfp );
		if ( !inside )
			break;
	}
	if ( inside )
		nfpList.pop_front();
	if ( logCallback && nfpList.empty() )
		logCallback( Error, std::format( "failed to generate nfp with minkowski sum for {0}, {1}, {2}", a.id(), b.id(),
										 inside ? "inside" : "outside" ) );
	return nfpList;
}
