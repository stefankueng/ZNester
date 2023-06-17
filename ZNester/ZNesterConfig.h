#pragma once

struct ZNesterConfig
{
	double partDistance = 2.0;	// distance between the parts that are nested
	double binDistance	= 5.0;	// distance to the bin border
	bool   useHoles		= true;

	// params for the genetic algorithm: only change if you know what you're doing
	size_t populationSize = 10;
	size_t mutationRate	  = 50;
};
