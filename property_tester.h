#pragma once

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cmath>

#include "nfa.h"

/***** Declarations *****/
// wrapper class for samples in the property testing algorithm.
class Sample;

// Tests efficiently whether a fragment is `nfa`-blocking.
// Subroutine for property_test.
template <typename T>
bool is_blocking(std::vector<Sample> &fragment, const Nfa<T> &nfa, int n);

// Main algorithm: `eps`-Property tester for regular languages.
// returns `true` if `u` belongs to L(automaton),
// or `false` with probability at least `1 - error_proba`
// if the edit distance of `u` to L(automaton) is at least `eps * |u|`.
// If none of the above holds (i.e. 0 < dist(u, L) < eps * |u|)
// it may return `true` or `false`.
// Details on how the algorithm works may be found in [Bathie and Starikovskaya, 2020]
template <typename T, typename Container = std::vector<T>>
bool property_test(Nfa<T> &automaton, const Container &u, double eps, double error_proba);

/***** Implementation *****/
class Sample
{
public:
	int start;
	int end;
	Sample(int s, int length): start(s), end(s + length) { };
};

template <typename T, typename Container = std::vector<T>>
bool is_blocking(std::vector<Sample> &fragment, const Container &u, const Nfa<T> &nfa, int n)
{
	std::sort(fragment.begin(), 
			  fragment.end(), 
			  [](const Sample &s, const Sample &t) { return s.start < t.start; });
	std::vector<bool> reachable = nfa.initial_states(); 

	int frag_i = 0, index = 0;
	while (index < n)
	{
		// If there are no more intervals in the fragment,
		// add one star and break.
		if (frag_i >= (int)fragment.size())
		{
			reachable = nfa.star_reach(reachable);
			break;
		}
		const Sample &cur_sample = fragment[frag_i];
		if (index < cur_sample.start)
		{
			reachable = nfa.star_reach(reachable);
			index = cur_sample.start;
		}
		else if (index < cur_sample.end)
		{
			reachable = nfa.letter_reach(reachable, u[index]);
			++index;	
		}
		else
		{
			++frag_i;
		}
	}
	
	for (int i = 0; i < nfa.num_states(); ++i)
		if (reachable[i] && nfa.is_final(i))
			return false;

	return true;
}


template <typename T, typename Container = std::vector<T>>
bool property_test(Nfa<T> &automaton, const Container &u, double eps, double error_proba)
{
	if (error_proba == 0 || eps == 0)
		throw std::logic_error("property_test: eps and error_proba must be non-zero.");
	
	int n = u.size();
	int k = automaton.num_scc();
	int m = automaton.num_states();
	double beta = eps/ (6*m);
	int gamma = std::ceil(2/ beta);

	if (n < std::max(3*gamma*std::ceil(std::log(gamma)), std::ceil(k / beta)))
		return automaton.accepts(u);

	std::vector<Sample> fragment;
	int lambda = std::ceil(2 * std::log(6 * k * std::pow(2, k) / error_proba) / beta);
	for (int i = 0; i < lambda; ++i)
		fragment.push_back(Sample(rand() % n, 1));

	for (int i = 0; i < std::ceil(std::log(gamma)); ++i)
	{
		int l = 1 << i;
		int alpha = std::ceil(3 * std::log(6 * k * std::pow(2, k) / error_proba) 
						 * gamma * std::ceil(std::log(gamma)) / l);
		for (int j = 0; j < alpha; ++j)
		{
			fragment.push_back(Sample(rand() % n, 2*l));		
		}
	}

	return !(is_blocking(fragment, u, automaton, n));
}