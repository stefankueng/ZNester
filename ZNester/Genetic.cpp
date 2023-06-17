#include "pch.h"

#include "Genetic.h"

#include <corecrt_math_defines.h>

#include <algorithm>
#include <random>

Genetic::Genetic( const ZPolygon& bin, const std::deque<ZPolygon>& adam, const ZNesterConfig& config )
{
	m_binBounds = bin.bounds();
	Individual individual;
	individual.placement = adam;

	// population is an array of individuals.
	// Each individual is an object representing the order of insertion and the angle each part is rotated
	for ( const auto& p : individual.placement )
	{
		individual.rotation.push_back( randomAngle( p ) );
	}
	m_population.push_back( individual );
	while ( m_population.size() < config.populationSize )
	{
		auto mutant = mutate( individual, config );
		m_population.push_back( mutant );
	}
}

std::deque<Individual>& Genetic::population()
{
	return m_population;
}

Individual Genetic::mutate( const Individual& individual, const ZNesterConfig& config ) const
{
	auto clone		  = individual;
	auto mutationRate = config.mutationRate * RAND_MAX / 100;
	for ( size_t i = 0; i < clone.placement.size() - 1; ++i )
	{
		auto rnd = rand();
		if ( rnd < mutationRate )
		{
			// swap current part with next part
			std::swap( clone.placement[i], clone.placement[i + 1] );
			std::swap( clone.rotation[i], clone.rotation[i + 1] );
		}

		rnd = rand();
		if ( rnd < mutationRate / 4 )
		{
			clone.rotation[i] = randomAngle( clone.placement[i] );
		}
	}
	return clone;
}

std::tuple<Individual, Individual> Genetic::mate( const Individual& male, const Individual& female )
{
	auto	   cutpoint = static_cast<__int64>( round(
		  std::min( std::max( static_cast<double>( rand() ) / RAND_MAX, 0.1 ), 0.9 ) * ( male.placement.size() - 1 ) ) );

	Individual child1;
	child1.placement = std::deque<ZPolygon>( male.placement.begin(), male.placement.begin() + cutpoint );
	child1.rotation	 = std::deque<double>( male.rotation.begin(), male.rotation.begin() + cutpoint );

	Individual child2;
	child2.placement = std::deque<ZPolygon>( female.placement.begin(), female.placement.begin() + cutpoint );
	child2.rotation	 = std::deque<double>( female.rotation.begin(), female.rotation.begin() + cutpoint );

	for ( size_t i = 0; i < female.placement.size(); ++i )
	{
		auto id	  = female.placement[i].id();
		auto copy = female.placement[i].copy();
		if ( std::ranges::find_if( child1.placement, [&]( const auto& poly )
								   { return id == poly.id() && copy == poly.copy(); } ) == child1.placement.end() )
		{
			child1.placement.push_back( female.placement[i] );
			child1.rotation.push_back( female.rotation[i] );
		}
	}

	for ( size_t i = 0; i < male.placement.size(); ++i )
	{
		auto id	  = male.placement[i].id();
		auto copy = male.placement[i].copy();
		if ( std::ranges::find_if( child2.placement, [&]( const auto& poly )
								   { return id == poly.id() && copy == poly.copy(); } ) == child2.placement.end() )
		{
			child2.placement.push_back( male.placement[i] );
			child2.rotation.push_back( male.rotation[i] );
		}
	}

	return std::make_tuple( child1, child2 );
}

void Genetic::generation( const ZNesterConfig& config )
{
	// Individuals with higher fitness are more likely to be selected for mating
	std::ranges::sort( m_population, [&]( const auto& p1, const auto& p2 ) { return p1.fitness - p2.fitness; } );

	// fittest individual is preserved in the new generation (elitism)
	std::deque<Individual> newPopulation;
	newPopulation.push_back( m_population[0] );

	while ( newPopulation.size() < m_population.size() )
	{
		const auto& male   = randomWeightedIndividual();
		const auto& female = randomWeightedIndividual( &male );

		// each mating produces two children
		auto children = mate( male, female );

		// slightly mutate children
		newPopulation.push_back( mutate( std::get<0>( children ), config ) );

		if ( newPopulation.size() < m_population.size() )
		{
			newPopulation.push_back( mutate( std::get<1>( children ), config ) );
		}
	}

	m_population = newPopulation;
}

const Individual& Genetic::randomWeightedIndividual( const Individual* exclude )
{
	auto   rnd	  = static_cast<double>( rand() ) / RAND_MAX;

	auto   lower  = 0.0;
	auto   weight = 1.0 / m_population.size();
	auto   upper  = weight;

	size_t i	  = 0;
	for ( const auto& individual : m_population )
	{
		// if the random number falls between lower and upper bounds, select this individual
		if ( rnd > lower && rnd < upper )
		{
			if ( &individual != exclude )
				return individual;
		}
		lower = upper;
		upper += 2 * weight * ( ( m_population.size() - i ) / m_population.size() );
		++i;
	}

	return m_population[0];
}

double Genetic::randomAngle( const ZPolygon& poly ) const
{
	static auto		   engine = std::default_random_engine{};
	std::deque<double> angleList;
	if ( poly.rotations() == 0 )
		angleList.push_back( poly.rotation() );
	else
	{
		for ( size_t i = 0; i < std::max( 1ULL, poly.rotations() ); ++i )
			angleList.push_back( i * 2 * M_PI / poly.rotations() );
	}

	std::ranges::shuffle( angleList, engine );

	for ( const auto& angle : angleList )
	{
		auto p = poly;
		p.rotate( angle );
		auto bounds = p.bounds();

		// don't use obviously bad angles where the part doesn't fit in the bin
		if ( bounds.width() < m_binBounds.width() && bounds.height() < m_binBounds.height() )
		{
			return angle;
		}
	}

	return 0.0;
}
