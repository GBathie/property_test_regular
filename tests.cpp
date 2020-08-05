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

// Dynamic programming to compute the edit distance
// of a word to a language.
// D[i,j] : smallest cost of edit of u[1,i] to a word
// that labels a run from an initial state to j
int edit_distance(const string &u, const Nfa<char> &a)
{
	int m = a.num_states();
	int n = u.size();
	// Compute underlying graph
	vector<vector<int>> graph(m, vector<int>(m, n));
	for (int i = 0; i < m; ++i)
	{
		for (int j = 0; j < m; ++j)
			if (a.is_transition(i, '0', j) || a.is_transition(i, '1', j))
					graph[i][j] = 1;
		graph[i][i] = 0;
	}
	// Floyd-Warshall for APSP
	for (int i = 0; i < m; ++i)
		for (int j = 0; j < m; ++j)
			for (int k = 0; k < m; ++k)
				graph[i][j] = min(graph[i][j], graph[i][k] + graph[k][j]);

	// Dynamic programming memory
	vector<vector<int>> D(n + 1, vector<int>(m, n + 1));
	// Base case
	for (int j = 0; j < m; ++j)
		for (int q = 0; q < m; ++q)
			if (a.is_initial(q))
				D[0][j] = min(D[0][j], graph[q][j]);

	// Compute content
	for (int i = 1; i < n+1; ++i)
	{
		for (int q = 0; q < m; ++q)
		{
			// Deletion
			D[i][q] = min(D[i][q], D[i-1][q] + 1);
			for (int p = 0; p < m; ++p)
			{
				// No edition
				if (a.is_transition(p, u[i-1], q))
					D[i][q] = min(D[i][q], D[i-1][p]);
				// Substitution
				if (a.is_transition(p, '0', q) || a.is_transition(p, '1', q))
					D[i][q] = min(D[i][q], D[i-1][p] + 1);
			}
			for (int p = 0; p < m; ++p)
				D[i][q] = min(D[i][q], D[i][p] + graph[p][q]);
			// Shortest number of insertions
		}
	}
	
	int mini = n;
	for (int i = 0; i < m; ++i)
		if (a.is_final(i))
			mini = min(mini, D[n][i]);			
	return mini;
}

void benchmark_precision(Nfa<char> &nfa, int n , int l, double eps, double p)
{	
	cout << n  << " ";
	cout << l  << " ";
	cout << eps << " ";
	cout << p  << " ";
	cout << endl;
	int a,b;
	for (int i = 0; i < n; ++i)
	{
		auto s = random_s(l);
		a = property_test(nfa, s, eps, p);
		b = nfa.accepts(s);
		cout << a << " " << b << " " << edit_distance(s, nfa) << endl;
	}
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
	
	benchmark_time("time01.txt", zero_star_one_star, 50, 50'000);
	auto nfa1 = random_nfa(10, .3, .1);
	benchmark_time("time_random.txt", nfa1, 50, 50'000);
	
	benchmark_precision(zero_star_one_star, 50, 50'000, 0.3, 0.3);
	
	// NFA for words with length that can be written 5n+2 for some integer n
	Nfa<char> length_5n_plus_2(5);
	for (int i = 0; i < 5; ++i)
		for (char c: {'0', '1'})
			length_5n_plus_2.add_transition(i, c, (i+1)%5);
	length_5n_plus_2.set_initial(0);
	length_5n_plus_2.set_final(2);
	benchmark_precision(length_5n_plus_2, 50, 50'000, 0.3, 0.3);
	

	// NFA for 1(0 + 1)^*
	Nfa<char> one_sigma_star(2);
	one_sigma_star.add_transition(0,'1', 1);
	one_sigma_star.add_transition(1,'0', 1);
	one_sigma_star.add_transition(1,'1', 1);
	one_sigma_star.set_initial(0);
	one_sigma_star.set_final(1);
	benchmark_precision(one_sigma_star, 50, 50'000, 0.3, 0.3);

	return 0;
}