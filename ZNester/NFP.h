#pragma once
#include "ZNester.h"

struct NfpKey
{
	size_t aId		 = 0;
	size_t bId		 = 0;
	bool   inside	 = false;
	double aRotation = 0.0;
	double bRotation = 0.0;
	bool   operator<( const NfpKey& o ) const
	{
		if ( aId != o.aId )
			return aId < o.aId;
		if ( bId != o.bId )
			return bId < o.bId;
		if ( inside != o.inside )
			return inside;
		if ( !dblEqual( aRotation, o.aRotation ) )
			return aRotation < o.aRotation;
		if ( !dblEqual( bRotation, o.bRotation ) )
			return bRotation < o.bRotation;
		return false;
	}
};

struct NfpPair
{
	const ZPolygon& partA;
	const ZPolygon& partB;
	NfpKey			key;
};

// returns true if point already exists in the given nfp
bool inNfp( const ZPoint& p, const std::deque<ZPolygon>& nfp );

// searches for an arrangement of A and B such that they do not overlap
// if an NFP is given, only search for startpoints that have not already been traversed in the given NFP
ZPoint searchStartPoint( ZPolygon& a, ZPolygon& b, bool inside, const std::deque<ZPolygon>& nfp,
						 const std::function<void( eZLogLevel, const std::string& msg )>& logCallback,
						 const tDebugCallback&											 debugDisplay );

// returns an interior NFP for the special case where A is a rectangle
// used if the bin is a rectangle
std::deque<ZPolygon> noFitPolygonRectangle( const ZPolygon& a, const ZPolygon& b );

// given a static polygon A and a movable polygon B, compute a no fit polygon by orbiting B around A
// if the inside flag is set, B is orbited inside of A rather than outside
// if the searchEdges flag is set, all edges of A are explored for NFPs.
// note: we don't need searchEdges because we treat holes in polygons as separate polygons
std::deque<ZPolygon> noFitPolygon( ZPolygon& a, ZPolygon& b, bool inside, bool searchEdges,
								   const std::function<void( eZLogLevel, const std::string& msg )>& logCallback,
								   const tDebugCallback&											debugDisplay );
void				 minkowskiFallback( std::deque<ZPolygon>&											 nfpList,
										const std::function<void( eZLogLevel, const std::string& msg )>& logCallback, const ZPolygon& a,
                                        const ZPolygon& b, bool inside, const tDebugCallback& debugDisplay, ZPolygon& nfp );
std::deque<ZPolygon> noFitPolygonMinkowski( const ZPolygon& a, const ZPolygon& b, bool inside,
											const std::function<void( eZLogLevel, const std::string& msg )>& logCallback,
											const tDebugCallback& debugDisplay );