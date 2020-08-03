#pragma once

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cmath>

#include "nfa.h"

/***** Declarations *****/
template <typename T> 
// wrapper for samples in the property testing algorithm.
class Sample;

// Tests efficiently whether a fragment is `nfa`-blocking.
// Subroutine for property_test.
template <typename T> 
bool is_blocking(std::vector<Sample<T>> &fragment, const Nfa<T> &nfa, int n);

// Main algorithm: `eps`-Property tester for regular languages.
// returns `true` if `u` belongs to L(automaton),
// or `false` with probability at least `1 - error_proba`
// if the edit distance of `u` to L(automaton) is at most `eps * |u|`.
// In the last case, it may return `true` or `false`.
template <typename T, typename Container = std::vector<T>>
bool property_test(Nfa<T> &automaton, const Container &u, double eps, double error_proba);

/***** Implementation *****/
template <typename T> 
class Sample
{
public:
	int start_index;
	std::vector<T> letters;
	inline int size() const { return letters.size(); };
	Sample(int i, const std::vector<T> &s): start_index(i), letters(s) { };

	template <typename Container = std::vector<T>>
	static Sample<T> from_position_length(int pos, int length, const Container &source)
	{
		std::vector<T> v(source.begin() + pos, source.begin() + pos + length);
		return Sample<T>(pos, v);
	}
};

template <typename T>
bool is_blocking(std::vector<Sample<T>> &fragment, const Nfa<T> &nfa, int n)
{
	std::sort(fragment.begin(), 
			  fragment.end(), 
			  [](const Sample<T> &s, const Sample<T> &t) { return s.start_index < t.start_index; });
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
		const Sample<T> &cur_sample = fragment[frag_i];
		if (index < cur_sample.start_index)
		{
			reachable = nfa.star_reach(reachable);
			index = cur_sample.start_index;
		}
		else if (index < cur_sample.start_index + cur_sample.size())
		{
			reachable = nfa.letter_reach(reachable, cur_sample.letters[index - cur_sample.start_index]);
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

	if (n < 12*gamma*std::ceil(std::log(gamma)))
		return automaton.accepts(u);

	std::vector<Sample<T>> fragment;
	int lambda = std::ceil(2 * std::log(6 * k * std::pow(2, k) / error_proba) / beta);
	for (int i = 0; i < lambda; ++i)
		fragment.push_back(Sample<T>::from_position_length(rand() % n, 1, u));

	for (int i = 0; i < std::ceil(std::log(gamma)); ++i)
	{
		int l = 1 << i;
		int alpha = std::ceil(3 * std::log(6 * k * std::pow(2, k) / error_proba) 
						 * gamma * std::ceil(std::log(gamma)) / l);
		for (int j = 0; j < alpha; ++j)
		{
			fragment.push_back(Sample<T>::from_position_length(rand() % n, 2*l, u));		
		}
	}

	return !(is_blocking<T>(fragment, automaton, n));
}