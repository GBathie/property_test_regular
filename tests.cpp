#include <chrono>
#include <string>
#include <iostream>
#include <fstream>
#include <functional>
#include <ctime>
#include <random>

#include "property_tester.h"
#include "nfa.h"

using namespace std;

string random_s(int l)
{
	string res;
	for (int i = 0; i < l; ++i)
		res += (rand()%2) ? '0' : '1';

	return res;
}

vector<string> random_inputs(int n, int l)
{
	vector<string> res;
	for (int i = 0; i < n; ++i)
		res.push_back(random_s(l));

	return res;
}

int time_function(const vector<string> &inputs, function<bool(const string&)> f)
{
	auto t1 = chrono::high_resolution_clock::now();
	for (auto &s: inputs)
		f(s);

	auto t2 = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
	return duration;
}

template <typename T>
int time_approx(const vector<string> &inputs, Nfa<T> &automaton, double eps, double err_proba)
{
	return time_function(inputs, 
						 [&](const string &s) { return property_test(automaton, s, eps, err_proba); });	
}

template <typename T>
int time_exact(const vector<string> &inputs, const Nfa<T> &automaton)
{
	return time_function(inputs, 
						 [&](const string &s) { return automaton.accepts(s); });	
}

void benchmark_time(string fname, Nfa<char> &nfa, int n, int MAX_L)
{
	ofstream time_file(fname);
	time_file << "l exact approx05,03 approx03,03 approx01,01" << endl;
	for (int l = 1; l < MAX_L; l *= 2)
	{
		auto inputs = random_inputs(n, l);
		time_file << l << " " << time_exact(inputs, nfa);
		time_file << " " << time_approx(inputs, nfa, 0.5, 0.3); 
		time_file << " " << time_approx(inputs, nfa, 0.3, 0.3);
		time_file << " " << time_approx(inputs, nfa, 0.1, 0.1);
		time_file << endl; 
	}
}

Nfa<char> random_nfa(int n_states, double p, double p2)
{
	random_device rd;
	mt19937 mt(rd());
	uniform_real_distribution<double> ud(0,1);
	Nfa<char> res(n_states);
	for (int i = 0; i < n_states; ++i)
		for (int j = 0; j < n_states; ++j)
			if (i != j && ud(mt) < p)
				res.add_transition(i, ((ud(mt) < 0.5) ? '0': '1'),j);

	for (int i = 0; i < n_states; ++i)
	{
		if (ud(mt) < p2)
			res.set_initial(i);
		if (ud(mt) < p2)
			res.set_final(i);
	}
	return res;
}

int main()
{
	srand(time(nullptr));
	// Automaton for 0^* 1^*
	Nfa<char> zero_star_one_star(2); 
	zero_star_one_star.add_transition(0,'0', 0);
	zero_star_one_star.add_transition(0,'1', 1);
	zero_star_one_star.add_transition(1,'1', 1);
	zero_star_one_star.set_initial(0);
	zero_star_one_star.set_final(1);
	
	benchmark_time("time01.txt", zero_star_one_star, 50, 4'000'000);
	auto nfa1 = random_nfa(10, .3, .1);
	benchmark_time("time_random.txt", nfa1, 50, 10'000'000);
	// benchmark_precision(zero_star_one_star, 1000, 10'000, 0.49, 23);
	// benchmark_precision(nfa1, 10, 50'000, 0.3, 0.5);
	// auto nfa2 = random_nfa(50, .1, .1);
	// benchmark_precision(nfa2, 10, 20'000, 0.3, 0.5);

	return 0;
}