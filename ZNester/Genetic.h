#pragma once
#include "ZNesterConfig.h"
#include "ZPolygon.h"

struct Individual
{
	std::deque<ZPolygon> placement;
	std::deque<double>   rotation;
	double               fitness = 0.0;
};

class Genetic
{
public:
	Genetic( const ZPolygon& bin, const std::deque<ZPolygon>& adam, const ZNesterConfig& config );

	std::deque<Individual>&            population();
	Individual                         mutate( const Individual& individual, const ZNesterConfig& config ) const;
	std::tuple<Individual, Individual> mate( const Individual& male, const Individual& female );
	void                               generation( const ZNesterConfig& config );
	const Individual&                  randomWeightedIndividual( const Individual* exclude = nullptr );

private:
	double                 randomAngle( const ZPolygon& poly ) const;

	ZRect                  m_binBounds;
	std::deque<Individual> m_population;
};
