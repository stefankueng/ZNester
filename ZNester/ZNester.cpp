#include "pch.h"

#include "ZNester.h"

#include <corecrt_math_defines.h>
#include <intsafe.h>

#include <algorithm>
#include <execution>
#include <map>
#include <set>

#include "../Clipper2Lib/clipper.h"
#include "Genetic.h"
#include "NFP.h"
#include "ProfileTimer.h"

bool ZNester::doNest( const ZPolygon& binPoly, const std::deque<ZPolygon>& polygons, const ZNesterConfig& config )
{
	if ( !binPoly.isValid() )
	{
		if ( m_logCallback )
			m_logCallback( Error, "bin polygon is invalid" );
		return false;
	}
	if ( polygons.empty() )
	{
		if ( m_logCallback )
			m_logCallback( Error, "no polygons specified" );
		return false;
	}
	if ( !m_callBack )
	{
		if ( m_logCallback )
			m_logCallback( Error, "no callback function set" );
	}

	stopNest();
	m_fitness = DBL_MAX;

	srand( static_cast<int>( time( nullptr ) ) );

	m_bin = binPoly;
	m_bin.setId( SIZE_T_MAX );
	m_tree	 = buildTree( polygons );
	m_config = config;

	m_bin.shrink( m_config.binDistance );
	// remove duplicate endpoints, ensure counterclockwise winding direction
	if ( m_bin.area() > 0 )
		m_bin.reverse();
	if ( m_bin[0] == m_bin[m_bin.size() - 1] )
		m_bin.erase( m_bin.begin() );

	offsetTree( m_tree, -m_config.partDistance / 2.0 );
	for ( auto& p : m_tree )
	{
		if ( p.area() > 0 )
			p.reverse();
		if ( p[0] == p[p.size() - 1] )
			p.erase( p.begin() );
	}

	// enhancement: simplify the polygons and bin using a modified
	// Douglas-Pecker algorithm which ensures that the resulting polygon
	// is either always bigger (for polygons) or smaller (for holes)
	// example (in python) for 'always bigger' here:
	// https://github.com/prakol16/rdp-expansion-only

	// put the bin in the origin
	auto binBounds = m_bin.bounds();
	m_bin.translate( -binBounds.x(), -binBounds.y() );

	// sort the polygons by size for initial placement, biggest first
	std::ranges::sort( m_tree,
					   []( const auto& p1, const auto& p2 ) -> bool
					   { return abs( p1.bounds().area() ) > abs( p2.bounds().area() ); } );

	m_thread = std::thread(
		[&]()
		{
			srand( static_cast<int>( time( nullptr ) ) );
			m_run = true;
			runNesting( m_bin, m_tree, m_config );
		} );

	return true;
}

bool ZNester::stopNest()
{
	if ( m_run )
	{
		m_run = false;
		m_thread.join();
	}
	return true;
}

bool ZNester::isNesting() const
{
	return m_run;
}

bool ZNester::runNesting( const ZPolygon& binPoly, const std::deque<ZPolygon>& polygons, const ZNesterConfig& config )
{
	Genetic								   genetic( binPoly, polygons, config );
	auto								   binPolyArea = std::abs( binPoly.area() );

	std::map<NfpKey, std::deque<ZPolygon>> nfpCache;

	// do nesting iterations until manually stopped
	do
	{
		auto [fitness, allPlacements] = nestGArandomRotations( config, binPoly, genetic, nfpCache );
		// auto [fitness, allPlacements] = nestGAbestRotation( config, binPoly, genetic, nfpCache );

		// report the nesting result:
		// only pass the placement if it's better than the previous one,
		// otherwise only notify about the iteration
		if ( fitness < m_fitness )
		{
			m_fitness = fitness;
			if ( m_callBack )
			{
				// adjust placements with the bin offset
				for ( auto& placement : allPlacements )
				{
					for ( auto& position : placement )
					{
						position.x += config.binDistance;
						position.y += config.binDistance;
					}
				}
				m_callBack( allPlacements, binPolyArea );
			}
		}
		else
		{
			if ( m_callBack )
				m_callBack( {}, binPolyArea );
		}

	} while ( m_run );

	return false;
}

std::deque<ZPolygon> ZNester::buildTree( std::deque<ZPolygon> polygons ) const
{
	// go through all polygons and determine whether a polygon is inside another
	// those inside others are treated as holes
	// note: for now, only one level of polygons/holes is supported, i.e. a hole
	// can not contain other polygons
	std::set<ZPolygon*> children;
	size_t				i		   = 0;
	size_t				polyPoints = 0;
	size_t				holes	   = 0;
	for ( auto& poly : polygons )
	{
		// ensure that each polygon does not have start and end point the same
		if ( poly[0] == poly[poly.size() - 1] )
			poly.erase( poly.begin() );

		bool   isChild = false;
		size_t j	   = 0;
		for ( auto& poly2 : polygons )
		{
			if ( i == j )
			{
				++j;
				continue;
			}
			if ( poly2.isPointInside( poly[0], false ) == ePointInside::Inside )
			{
				poly2.addChild( poly );
				isChild = true;
				holes++;
				break;
			}
			++j;
		}
		if ( isChild )
			children.insert( &poly );
		++i;
		polyPoints += poly.size();
	}
	std::erase_if( polygons, [&]( auto& poly ) -> bool { return children.contains( &poly ); } );

	// insert copies
	std::deque<ZPolygon> copies;
	for ( const auto& poly : polygons )
	{
		if ( poly.copies() > 1 )
		{
			for ( size_t j = 1; j < poly.copies(); ++j )
			{
				copies.push_back( poly );
				auto& copy	= copies.back();
				copy.m_copy = j;
				if ( !copy.children().empty() )
				{
					for ( auto& child : copy.children() )
						child.m_copy = j;
				}
			}
		}
	}
	polygons.insert( polygons.end(), copies.begin(), copies.end() );

	if ( m_logCallback )
		m_logCallback( Info, std::format( "{0} polygons, {1} holes and {2} points in total", polygons.size(), holes,
										  polyPoints ) );
	return polygons;
}

void ZNester::offsetTree( std::deque<ZPolygon>& polygons, double offset )
{
	for ( auto& poly : polygons )
	{
		poly.shrink( offset );
		if ( !poly.children().empty() )
		{
			offsetTree( poly.children(), -offset );
		}
	}
}

std::deque<ZPolygon> ZNester::generateNfps( ZPolygon& a, ZPolygon& b, const NfpKey& key, bool useHoles ) const
{
	std::deque<ZPolygon> nfPs;

	if ( key.inside )
	{
		if ( a.isRectangle() )
			nfPs = noFitPolygonRectangle( a, b );
		else
			nfPs = noFitPolygon( a, b, true, false, m_logCallback, m_debugDisplay );

		// ensure all interior NFPs have the same winding direction
		if ( !nfPs.empty() )
		{
			for ( size_t i = 0; i < nfPs.size(); ++i )
			{
				if ( nfPs[i].area() > 0 )
				{
					nfPs[i].reverse();
				}
			}
		}
		else
		{
			// warning on null inner NFP
			// this is not an error, as the part may simply be larger
			// than the bin or otherwise unplaceable due to geometry
			if ( m_logCallback )
				m_logCallback( Warning, std::format( "null inner NFP, part is too big to fit in bin: {0}", key.bId ) );
		}
	}
	else
	{
		nfPs = noFitPolygon( a, b, false, false, m_logCallback, m_debugDisplay );
		// sanity check
		if ( nfPs.empty() )
		{
			if ( m_logCallback )
				m_logCallback( Error, std::format( "failed to generate NFP for parts {0} and {1}", key.aId, key.bId ) );
			return nfPs;
		}

		// for outer NFPs, the first is guaranteed to be the largest.
		// Any subsequent NFPs that lie inside the first are holes
		for ( size_t i = 0; i < nfPs.size(); ++i )
		{
			if ( nfPs[i].area() > 0 )
			{
				nfPs[i].reverse();
			}

			if ( i > 0 )
			{
				if ( nfPs[0].isPointInside( nfPs[i][0], false ) == ePointInside::Inside )
				{
					if ( nfPs[i].area() < 0 )
					{
						nfPs[i].reverse();
					}
				}
			}
		}

		// generate nfps for children (holes of parts) if any exist
		if ( useHoles && !a.children().empty() )
		{
			auto Bbounds = b.bounds();

			for ( size_t i = 0; i < a.children().size(); ++i )
			{
				auto aBounds = a.children()[i].bounds();

				// no need to find nfp if B's bounding box is too big
				if ( aBounds.width() > Bbounds.width() && aBounds.height() > Bbounds.height() )
				{
					auto cnfp = noFitPolygon( a.children()[i], b, true, false, m_logCallback, m_debugDisplay );
					// ensure all interior NFPs have the same winding direction
					if ( !cnfp.empty() )
					{
						for ( size_t j = 0; j < cnfp.size(); ++j )
						{
							if ( cnfp[j].area() < 0 )
							{
								cnfp[j].reverse();
							}
							nfPs.push_back( cnfp[j] );
						}
					}
				}
			}
		}
	}
	return nfPs;
}

std::deque<ZPolygon> ZNester::getCombinedNfp( const ZPolygon& path, const std::deque<ZPolygon>& placed,
											  const std::deque<ZPolygon>&					binNfp,
											  const std::map<NfpKey, std::deque<ZPolygon>>& nfpCache,
											  const ZPlacement&								placements ) const
{
	Clipper2Lib::PathsD clipperPaths;
	for ( size_t j = 0; j < placed.size(); ++j )
	{
		NfpKey placedKey{ placed[j].id(), path.id(), false, placed[j].rotation(), path.rotation() };
		if ( !nfpCache.contains( placedKey ) )
			continue;

		const auto& nfp = nfpCache.at( placedKey );
		for ( const auto& nfpPoly : nfp )
		{
			Clipper2Lib::PathD clipperPath;
			clipperPath.reserve( nfpPoly.size() );
			for ( const auto& pt : nfpPoly )
			{
				clipperPath.emplace_back( ( pt.x() + placements[j].x ) * CLIPPER_LIB_SCALE,
										  ( pt.y() + placements[j].y ) * CLIPPER_LIB_SCALE );
			}
			clipperPaths.push_back( clipperPath );
		}
	}
	auto combinedNfp = Clipper2Lib::Union( clipperPaths, Clipper2Lib::FillRule::NonZero );
	if ( combinedNfp.empty() )
		return {};

	Clipper2Lib::PathsD clipperBinNfp;
	for ( const auto& nfpPoly : binNfp )
	{
		Clipper2Lib::PathD clipperPath;
		clipperPath.reserve( nfpPoly.size() );
		for ( const auto& pt : nfpPoly )
		{
			clipperPath.emplace_back( pt.x() * CLIPPER_LIB_SCALE, pt.y() * CLIPPER_LIB_SCALE );
		}
		clipperBinNfp.push_back( clipperPath );
	}

	auto finalClipperNfp = Clipper2Lib::Difference( clipperBinNfp, combinedNfp, Clipper2Lib::FillRule::NonZero );
	if ( finalClipperNfp.empty() )
		return {};

	std::deque<ZPolygon> finalNfp;
	for ( const auto& clipperPoly : finalClipperNfp )
	{
		ZPolygon poly;
		for ( const auto& pt : clipperPoly )
		{
			poly.emplace_back( pt.x / CLIPPER_LIB_SCALE, pt.y / CLIPPER_LIB_SCALE );
		}
		finalNfp.push_back( poly );
	}
	return finalNfp;
}

std::tuple<double, std::deque<ZPlacement>> ZNester::nestGArandomRotations(
	const ZNesterConfig& config, const ZPolygon& binPoly, Genetic& genetic,
	std::map<NfpKey, std::deque<ZPolygon>>& nfpCache )
{
	Individual* individual = nullptr;
	// evaluate all members of the population
	for ( size_t i = 0; i < genetic.population().size(); ++i )
	{
		if ( !genetic.population()[i].fitness )
		{
			individual = &genetic.population()[i];
			break;
		}
	}

	if ( individual == nullptr )
	{
		// all individuals have been evaluated, start next generation
		genetic.generation( config );
		individual = &genetic.population()[1];
	}

	auto  binPolyArea = std::abs( binPoly.area() );

	auto& placelist	  = individual->placement;
	auto& rotations	  = individual->rotation;

	for ( size_t i = 0; i < placelist.size(); ++i )
	{
		placelist[i].setRotation( rotations[i] );
	}

	// determine which nfps need to be calculated
	std::deque<NfpPair> nfpPairs;
	std::set<NfpKey>	nfpPairKeys;
	for ( size_t i = 0; i < placelist.size(); ++i )
	{
		auto&  part = placelist[i];
		NfpKey key	= { binPoly.id(), part.id(), true, 0.0, rotations[i] };
		if ( !nfpCache.contains( key ) && !nfpPairKeys.contains( key ) )
		{
			nfpPairs.emplace_back( binPoly, part, key );
			nfpPairKeys.insert( key );
		}
		for ( size_t j = 0; j < i; ++j )
		{
			auto& placed = placelist[j];
			key			 = { placed.id(), part.id(), false, rotations[j], rotations[i] };
			if ( !nfpCache.contains( key ) && !nfpPairKeys.contains( key ) )
			{
				nfpPairs.emplace_back( placed, part, key );
				nfpPairKeys.insert( key );
			}
		}
	}

	// calculate all necessary nfps for this iteration
	std::mutex cacheMutex;
	std::for_each( std::execution::par, nfpPairs.begin(), nfpPairs.end(),
				   [&]( const auto& pair )
				   {
					   if ( m_run )
					   {
						   auto a	 = pair.partA.rotated( pair.key.aRotation );
						   auto b	 = pair.partB.rotated( pair.key.bRotation );
						   auto nfPs = generateNfps( a, b, pair.key, config.useHoles );
						   if ( !nfPs.empty() )
						   {
							   std::deque<ZPolygon> cleanNfps;
							   for ( size_t i = 0; i < nfPs.size(); ++i )
							   {
								   const auto& nfp = nfPs[i];

								   if ( !nfp.empty() )
								   {
									   // a null nfp means the nfp could not be generated,
									   // either because the parts simply don't fit or an error in the nfp algo
									   cleanNfps.push_back( nfp );
								   }
							   }
							   std::lock_guard lock( cacheMutex );
							   nfpCache[pair.key] = cleanNfps;
						   }
					   }
				   } );

	// place paths

	// rotate paths by given rotation
	std::deque<ZPolygon> rotated;
	for ( const auto& poly : placelist )
	{
		auto r = poly.rotated( poly.rotation() );
		r.setRotation( poly.rotation() );
		rotated.push_back( r );
	}

	auto&				   paths = rotated;

	std::deque<ZPlacement> allPlacements;
	double				   fitness = 0.0;
	ProfileTimer		   timer( L"placing" );
	while ( m_run && paths.size() > 0 )
	{
		std::deque<ZPolygon> placed;
		ZPolygon			 placedPoints;
		ZPlacement			 placements;
		ZPlacement			 childPlacements;
		fitness += 1.0;	 // add 1 for each new bin opened (lower fitness is better)
		double minWidth = DBL_MAX;

		for ( const auto& path : paths )
		{
			if ( !m_run )
				return {};

			// inner NFP
			NfpKey		binKey{ SIZE_T_MAX, path.id(), true, 0ULL, path.rotation() };
			const auto& binNfp = nfpCache[binKey];

			// part unplaceable, skip (will be placed into next bin)
			if ( binNfp.empty() )
				continue;

			// ensure all necessary NFPs exist
			bool unplaceable = false;
			for ( size_t j = 0; j < placed.size(); ++j )
			{
				NfpKey placedKey{ placed[j].id(), path.id(), false, placed[j].rotation(), path.rotation() };

				if ( !nfpCache.contains( placedKey ) )
				{
					unplaceable = true;
					break;
				}
			}

			// part unplaceable, skip (will be placed into next bin)
			if ( unplaceable )
				continue;

			ZPosition position;
			if ( placed.empty() )
			{
				// first placement, put it on the bottom left
				for ( size_t j = 0; j < binNfp.size(); ++j )
				{
					for ( size_t k = 0; k < binNfp[j].size(); ++k )
					{
						if ( position.x == DBL_MAX || binNfp[j][k].x() - path[0].x() < position.x ||
							 ( dblEqual( binNfp[j][k].x() - path[0].x(), position.x ) &&
							   binNfp[j][k].y() - path[0].y() < position.y ) )
						{
							position.x		  = binNfp[j][k].x() - path[0].x();
							position.y		  = binNfp[j][k].y() - path[0].y();
							position.id		  = path.id();
							position.rotation = path.rotation();
						}
					}
				}

				placements.push_back( position );
				placed.push_back( path );
				for ( const auto& pt : path )
					placedPoints.emplace_back( pt.x() + position.x, pt.y() + position.y );
				if ( !path.children().empty() )
				{
					for ( const auto& child : path.children() )
					{
						auto childPos = position;
						childPos.id	  = child.id();
						childPlacements.push_back( childPos );
					}
				}

				continue;
			}

			// get the nfp of all places polygons against the one we have to place now
			auto finalNfp = getCombinedNfp( path, placed, binNfp, nfpCache, placements );

			// choose placement that results in the smallest convex hull

			minWidth		   = DBL_MAX;
			double minArea	   = DBL_MAX;
			double minx		   = DBL_MAX;
			double minHullArea = DBL_MAX;
			ZRect  minBounds;
			for ( const auto& nfPoly : finalNfp )
			{
				double allMinX = DBL_MAX;
				double allMaxX = -DBL_MAX;
				double allMinY = DBL_MAX;
				double allMaxY = -DBL_MAX;

				for ( size_t m = 0; m < placed.size(); ++m )
				{
					for ( size_t n = 0; n < placed[m].size(); ++n )
					{
						allMinX = std::min( allMinX, placed[m][n].x() + placements[m].x );
						allMaxX = std::max( allMaxX, placed[m][n].x() + placements[m].x );
						allMinY = std::min( allMinY, placed[m][n].y() + placements[m].y );
						allMaxY = std::max( allMaxY, placed[m][n].y() + placements[m].y );
					}
				}
				for ( size_t k = 0; k < nfPoly.size(); ++k )
				{
					// go through all points of the nfp and calculate how good the placement at
					// that point is

					// enhancement: if it's necessary to place a polygon only on a specific
					// raster grid, here's the place to do that

					// enhancement: currently only points of nfps are checked. If two subsequent
					// points are a large distance apart, we don't check the many possible placement
					// positions in between those points. Not sure if that would improve the final
					// placement though, but I leave this comment here...

					double	  allMinXTotal = allMinX;
					double	  allMaxXTotal = allMaxX;
					double	  allMinYTotal = allMinY;
					double	  allMaxYTotal = allMaxY;

					ZPosition shiftVector  = { nfPoly[k].x() - path[0].x(), nfPoly[k].y() - path[0].y(), path.id(),
											   path.rotation() };

					for ( const auto& pt : path )
					{
						allMinXTotal = std::min( allMinXTotal, pt.x() + shiftVector.x );
						allMaxXTotal = std::max( allMaxXTotal, pt.x() + shiftVector.x );
						allMinYTotal = std::min( allMinYTotal, pt.y() + shiftVector.y );
						allMaxYTotal = std::max( allMaxYTotal, pt.y() + shiftVector.y );
					}

					auto rectBounds =
						ZRect( allMinXTotal, allMinYTotal, allMaxXTotal - allMinXTotal, allMaxYTotal - allMinYTotal );

					auto allPoints = placedPoints;
					for ( const auto& pt : path )
						allPoints.emplace_back( pt.x() + shiftVector.x, pt.y() + shiftVector.y );
					auto hull = ZPolygon::convexHull( allPoints );
					auto hullArea = std::abs( hull.area() );

					// weigh width more, to help compress in direction of gravity
					auto area = hullArea * ( rectBounds.width() * 2.0 / rectBounds.height() );

					if ( area < minArea || ( dblEqual( minArea, area ) && ( shiftVector.x < minx ) ) )
					{
						minArea		= area;
						minWidth	= rectBounds.width();
						position	= shiftVector;
						minx		= shiftVector.x;
						minBounds	= rectBounds;
						minHullArea = hullArea;
					}
				}
			}
			if ( position.x != DBL_MAX )
			{
				// we found a position to place this polygon
				placed.push_back( path );
				for ( const auto& pt : path )
					placedPoints.emplace_back( pt.x() + position.x, pt.y() + position.y );
				placedPoints = ZPolygon::convexHull( placedPoints );
				placements.push_back( position );
				placements.bounds	= minBounds;
				placements.hullArea = minHullArea;
				if ( !path.children().empty() )
				{
					for ( const auto& child : path.children() )
					{
						auto childPos = position;
						childPos.id	  = child.id();
						childPlacements.push_back( childPos );
					}
				}
			}
			// if ( m_debugDisplay )
			//{
			//	if ( m_callBack )
			//	{
			//		auto debugPlacements = placements;
			//		if ( !childPlacements.empty() )
			//			debugPlacements.insert( debugPlacements.end(), childPlacements.begin(),
			//									childPlacements.end() );
			//		m_callBack( { debugPlacements }, binPolyArea );
			//	}
			// }
		}

		if ( minWidth != DBL_MAX )
		{
			fitness += minWidth / binPolyArea;
		}

		for ( size_t i = 0; i < placed.size(); ++i )
		{
			// remove the placed polygons from the global list,
			// what's left needs to be placed in the next bin
			paths.erase( std::ranges::remove_if( paths,
												 [&]( const auto& p ) -> bool
												 { return p.id() == placed[i].id() && p.copy() == placed[i].copy(); } )
							 .begin(),
						 paths.end() );
		}

		if ( !placements.empty() )
		{
			if ( !childPlacements.empty() )
				placements.insert( placements.end(), childPlacements.begin(), childPlacements.end() );
			allPlacements.push_back( placements );
		}
		else
		{
			if ( m_logCallback )
				m_logCallback( Error, "something went terribly wrong, stopping iteration" );
			break;
		}
	}

	// if paths is not empty, then some paths could not be placed
	// so increase the fitness
	fitness += 2 * paths.size();

	if ( !allPlacements.empty() )
		individual->fitness = fitness;

	return std::make_tuple( fitness, allPlacements );
}

// an attempt at placing polygons by trying each rotation at once
// note: doesn't work (yet) and is much slower than the nestGArandomRotations()
// algorithm. Leaving here as a reference in case we one day decide to try and improve
// on this.
std::tuple<double, std::deque<ZPlacement>> ZNester::nestGAbestRotation(
	const ZNesterConfig& config, const ZPolygon& binPoly, Genetic& genetic,
	std::map<NfpKey, std::deque<ZPolygon>>& nfpCache )
{
	Individual* individual = nullptr;
	// evaluate all members of the population
	for ( size_t i = 0; i < genetic.population().size(); ++i )
	{
		if ( !genetic.population()[i].fitness )
		{
			individual = &genetic.population()[i];
			break;
		}
	}

	if ( individual == nullptr )
	{
		// all individuals have been evaluated, start next generation
		genetic.generation( config );
		individual = &genetic.population()[1];
	}

	auto				   binPolyArea = std::abs( binPoly.area() );

	auto&				   placelist   = individual->placement;

	std::deque<ZPolygon>   placed;
	double				   fitness = 0.0;
	ZPlacement			   placements;
	ZPlacement			   childPlacements;
	std::deque<ZPlacement> allPlacements;
	while ( !placelist.empty() )
	{
		size_t placedCount = 0;
		double minWidth	   = DBL_MAX;
		for ( const auto& polyToPlace : placelist )
		{
			++placedCount;
			auto outStr = std::format( "handling {0} of {1}\n", placedCount, placelist.size() );
			OutputDebugStringA( outStr.c_str() );

			std::deque<double> angleList;
			if ( polyToPlace.rotations() == 0 )
				angleList.push_back( polyToPlace.rotation() );
			else
			{
				for ( size_t i = 0; i < std::max( 1ULL, polyToPlace.rotations() ); ++i )
					angleList.push_back( i * 2 * M_PI / polyToPlace.rotations() );
			}
			std::deque<NfpPair> nfpPairs;

			for ( const auto& angle : angleList )
			{
				NfpKey key = { binPoly.id(), polyToPlace.id(), true, 0.0, angle };
				if ( !nfpCache.contains( key ) )
				{
					nfpPairs.emplace_back( binPoly, polyToPlace, key );
				}
			}

			for ( const auto& placedPart : placed )
			{
				for ( const auto& angle : angleList )
				{
					NfpKey key = { placedPart.id(), polyToPlace.id(), false, placedPart.rotation(), angle };
					if ( !nfpCache.contains( key ) )
					{
						nfpPairs.emplace_back( placedPart, polyToPlace, key );
					}
				}
			}
			std::mutex cacheMutex;

			std::for_each( std::execution::seq, nfpPairs.begin(), nfpPairs.end(),
						   [&]( const auto& pair )
						   {
							   auto a	 = pair.partA.rotated( pair.key.aRotation );
							   auto b	 = pair.partB.rotated( pair.key.bRotation );
							   auto nfPs = generateNfps( a, b, pair.key, config.useHoles );
							   if ( !nfPs.empty() )
							   {
								   std::deque<ZPolygon> cleanNfps;
								   for ( size_t i = 0; i < nfPs.size(); ++i )
								   {
									   const auto& nfp = nfPs[i];

									   if ( !nfp.empty() )
									   {
										   // a null nfp means the nfp could not be generated,
										   // either because the parts simply don't fit or an error in the nfp algo
										   cleanNfps.push_back( nfp );
									   }
								   }
								   static bool debugDisplay = false;
								   if ( m_debugDisplay && debugDisplay )
									   m_debugDisplay( cleanNfps, {} );
								   std::lock_guard lock( cacheMutex );
								   nfpCache[pair.key] = cleanNfps;
							   }
						   } );

			// find the best angle
			ZPosition position;
			ZRect	  minBounds;
			ZPolygon  path = polyToPlace;
			minWidth	   = DBL_MAX;
			double minArea = DBL_MAX;
			double minx	   = DBL_MAX;
			for ( const auto& angle : angleList )
			{
				NfpKey binKey{ SIZE_T_MAX, polyToPlace.id(), true, 0ULL, angle };
				path = polyToPlace.rotated( angle );
				path.setRotation( angle );
				if ( !nfpCache.contains( binKey ) )
				{
					auto bPoly		 = binPoly;
					auto nfPs		 = generateNfps( bPoly, path, binKey, config.useHoles );
					nfpCache[binKey] = nfPs;
				}
				const auto& binNfp	 = nfpCache[binKey];

				auto		finalNfp = getCombinedNfp( path, placed, binNfp, nfpCache, placements );
				if ( finalNfp.empty() )
				{
					if ( placed.empty() )
					{
						finalNfp = binNfp;
					}
				}
				// choose placement that results in the smallest bounding box
				// could use convex hull instead, but it can create oddly shaped nests
				// (triangles or long slivers) which are not optimal for real-world use

				static bool debugDisplay = false;
				if ( m_debugDisplay && debugDisplay )
					m_debugDisplay( finalNfp, {} );

				for ( const auto& nfPoly : finalNfp )
				{
					double allMinX = DBL_MAX;
					double allMaxX = -DBL_MAX;
					double allMinY = DBL_MAX;
					double allMaxY = -DBL_MAX;

					for ( size_t m = 0; m < placed.size(); ++m )
					{
						for ( size_t n = 0; n < placed[m].size(); ++n )
						{
							allMinX = std::min( allMinX, placed[m][n].x() + placements[m].x );
							allMaxX = std::max( allMaxX, placed[m][n].x() + placements[m].x );
							allMinY = std::min( allMinY, placed[m][n].y() + placements[m].y );
							allMaxY = std::max( allMaxY, placed[m][n].y() + placements[m].y );
						}
					}
					for ( size_t k = 0; k < nfPoly.size(); ++k )
					{
						double	  allMinXTotal = allMinX;
						double	  allMaxXTotal = allMaxX;
						double	  allMinYTotal = allMinY;
						double	  allMaxYTotal = allMaxY;

						ZPosition shiftVector  = { nfPoly[k].x() - path[0].x(), nfPoly[k].y() - path[0].y(), path.id(),
												   path.rotation() };

						for ( const auto& pt : path )
						{
							allMinXTotal = std::min( allMinXTotal, pt.x() + shiftVector.x );
							allMaxXTotal = std::max( allMaxXTotal, pt.x() + shiftVector.x );
							allMinYTotal = std::min( allMinYTotal, pt.y() + shiftVector.y );
							allMaxYTotal = std::max( allMaxYTotal, pt.y() + shiftVector.y );
						}

						auto rectBounds = ZRect( allMinXTotal, allMinYTotal, allMaxXTotal - allMinXTotal,
												 allMaxYTotal - allMinYTotal );

						// weigh width more, to help compress in direction of gravity
						auto area = rectBounds.width() * 2.0 + rectBounds.height();

						if ( area < minArea || ( dblEqual( minArea, area ) && ( shiftVector.x < minx ) ) )
						{
							minArea	  = area;
							minWidth  = rectBounds.width();
							position  = shiftVector;
							minx	  = shiftVector.x;
							minBounds = rectBounds;
						}
					}
				}
			}
			if ( position.x != DBL_MAX )
			{
				ZPolygon bestPath = polyToPlace;
				bestPath.setRotation( position.rotation );
				placed.push_back( bestPath );
				placements.push_back( position );
				placements.bounds = minBounds;
				if ( !bestPath.children().empty() )
				{
					for ( const auto& child : bestPath.children() )
					{
						auto childPos = position;
						childPos.id	  = child.id();
						childPlacements.push_back( childPos );
					}
				}
			}
			if ( m_debugDisplay )
			{
				if ( m_callBack )
				{
					auto debugPlacements = placements;
					if ( !childPlacements.empty() )
						debugPlacements.insert( debugPlacements.end(), childPlacements.begin(), childPlacements.end() );
					m_callBack( { debugPlacements }, binPolyArea );
				}
			}
		}
		if ( minWidth )
		{
			fitness += minWidth / binPolyArea;
		}

		for ( size_t i = 0; i < placed.size(); ++i )
		{
			placelist.erase(
				std::ranges::remove_if( placelist,
										[&]( const auto& p ) -> bool
										{ return p.id() == placed[i].id() && p.copy() == placed[i].copy(); } )
					.begin(),
				placelist.end() );
		}

		if ( !placements.empty() )
		{
			if ( !childPlacements.empty() )
				placements.insert( placements.end(), childPlacements.begin(), childPlacements.end() );
			allPlacements.push_back( placements );
		}
		else
		{
			if ( m_logCallback )
				m_logCallback( Error, "something went terribly wrong, stopping iteration" );
			break;	// something went wrong
		}
	}
	// there were parts that couldn't be placed
	fitness += 2 * placelist.size();

	if ( !allPlacements.empty() )
		individual->fitness = fitness;

	return std::make_tuple( fitness, allPlacements );
}
