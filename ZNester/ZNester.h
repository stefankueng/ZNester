#pragma once
#include <map>
#include <thread>

#include "ZNesterConfig.h"
#include "ZPolygon.h"
#include "functional"

class Genetic;

struct ZPosition
{
	double x		= DBL_MAX;
	double y		= DBL_MAX;
	size_t id		= 0;
	double rotation = 0.0;
	double fitness	= 0.0;
};

struct ZPlacement : std::deque<ZPosition>
{
	double fitness = 0.0;
	ZRect  bounds;
	double hullArea = 0.0;
};

enum eZLogLevel : int
{
	Debug,
	Info,
	Warning,
	Error,
};

struct NfpKey;

using tDebugCallback = std::function<void( const std::deque<ZPolygon>&, const std::deque<ZPoint>& )>;

class ZNester
{
public:
	ZNester() = default;

	bool doNest( const ZPolygon& binPoly, const std::deque<ZPolygon>& polygons, const ZNesterConfig& config );
	bool stopNest();
	bool isNesting() const;
	void setCallback( const std::function<void( const std::deque<ZPlacement>&, double )> f )
	{
		m_callBack = f;
	}
	void setDebugDisplayCallback( const tDebugCallback& f )
	{
		m_debugDisplay = f;
	}
	void setLogCallback( const std::function<void( eZLogLevel level, const std::string& msg )> f )
	{
		m_logCallback = f;
	}

private:
	bool runNesting( const ZPolygon& binPoly, const std::deque<ZPolygon>& polygons, const ZNesterConfig& config );
	std::deque<ZPolygon> buildTree( std::deque<ZPolygon> polygons ) const;
	static void			 offsetTree( std::deque<ZPolygon>& polygons, double offset );
	std::deque<ZPolygon> generateNfps( ZPolygon& a, ZPolygon& b, const NfpKey& key, bool useHoles ) const;
	std::deque<ZPolygon> getCombinedNfp( const ZPolygon& path, const std::deque<ZPolygon>& placed,
										 const std::deque<ZPolygon>&				   binNfp,
										 const std::map<NfpKey, std::deque<ZPolygon>>& nfpCache,
										 const ZPlacement&							   placements ) const;
	std::tuple<double, std::deque<ZPlacement>> nestGArandomRotations(
		const ZNesterConfig& config, const ZPolygon& binPoly, Genetic& genetic,
		std::map<NfpKey, std::deque<ZPolygon>>& nfpCache );
	std::tuple<double, std::deque<ZPlacement>> nestGAbestRotation( const ZNesterConfig& config, const ZPolygon& binPoly,
																   Genetic&								   genetic,
																   std::map<NfpKey, std::deque<ZPolygon>>& nfpCache );

	tDebugCallback							   m_debugDisplay;
	std::function<void( const std::deque<ZPlacement>&, double )> m_callBack;
	std::function<void( eZLogLevel, const std::string& )>		 m_logCallback;

	double														 m_fitness = DBL_MAX;
	std::atomic_bool											 m_run	   = false;
	std::thread													 m_thread;
	ZPolygon													 m_bin;
	std::deque<ZPolygon>										 m_tree;
	ZNesterConfig												 m_config;
};
